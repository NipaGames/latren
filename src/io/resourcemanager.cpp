#include <latren/io/resourcemanager.h>
#include <latren/io/files/cfg.h>
#include <latren/io/paths.h>
#include <latren/io/serializablestruct.h>
#include <latren/game.h>

#include <fstream>

typedef std::string ResourceName;

const std::fs::path& Resources::GetDefaultPath(Resources::ResourceType t) {
    return Paths::RESOURCE_DIRS.at(t);
}
namespace Resources {
    TextureManager::TextureManager() : ResourceManager<Texture::TextureID>(GetDefaultPath(ResourceType::TEXTURE), ResourceName("texture")) { }
    ShaderManager::ShaderManager() : ResourceManager<GLuint>(GetDefaultPath(ResourceType::SHADER)) { }
    FontManager::FontManager() : ResourceManager<UI::Text::Font>(GetDefaultPath(ResourceType::FONT), ResourceName("font")) { }
    ModelManager::ModelManager() : ResourceManager<Model>(GetDefaultPath(ResourceType::MODEL), ResourceName("model")) { }
    StageManager::StageManager() : ResourceManager<Stage>(GetDefaultPath(ResourceType::STAGE), ResourceName("stage")) { }
};

void Resources::SaveConfig(const std::fs::path& path, const SerializableStruct& config) {
    Serializer::CFGSerializer serializer = Serializer::CFGSerializer();
    serializer.SetData(config.CFGSerialize());
    serializer.SerializeFile(path.string());
}

void Resources::LoadConfig(const std::fs::path& path, SerializableStruct& config) {
    Serializer::CFGSerializer serializer = Serializer::CFGSerializer();
    serializer.DeserializeFile(path.string());
    auto templateFields = config.CreateCFGTemplate();
    if (serializer.Success() && serializer.Validate(templateFields)) {
        config.CFGDeserialize(serializer.GetData());
    }
    else {
        spdlog::info("Restoring defaults...");
        SaveConfig(path, config);
    }
}

void ResourceManager::LoadConfigs() {
    if (!std::fs::is_directory(Paths::USER_DIR) || !std::fs::exists(Paths::USER_DIR))
        std::fs::create_directory(Paths::USER_DIR);
    if (!std::fs::is_directory(Paths::SAVEDATA_DIR) || !std::fs::exists(Paths::SAVEDATA_DIR))
        std::fs::create_directory(Paths::SAVEDATA_DIR);
    
    Resources::LoadConfig(Paths::VIDEO_SETTINGS_PATH, videoSettings);
}

Resources::Imports<Resources::Import> Resources::ListImports(const CFG::CFGField<std::vector<CFG::ICFGField*>>* obj, ResourceType t) {
    using namespace CFG;
    Imports<Import> imports = Imports<Import>(t);

    if (obj == nullptr)
        return imports;
    if (obj->name.has_value())
        imports.parentPath = Paths::Path(Paths::RESOURCES_DIR, obj->name.value());
    const std::vector<ICFGField*>& items = obj->GetItems();
    for (const ICFGField* v : items) {
        if (v->type != CFGFieldType::STRUCT) {
            spdlog::warn("'{}' cannot be parsed as a list of imports!", obj->name.value_or(""));
            return imports;
        }
    }
    for (const ICFGField* v : items) {
        const CFGObject* import = static_cast<const CFGObject*>(v);
        if (import == nullptr)
            continue;
        const auto* pathField = import->GetItemByIndex<std::string>(0);
        if (pathField == nullptr)
            continue;
        Resources::Import importStruct;
        importStruct.id = import->name.value_or("");
        importStruct.path = pathField->GetValue();
        for (int i = 1; i < import->GetItems().size(); i++) {
            const ICFGField* additional = import->GetItemByIndex(i);
            if (additional->automaticallyCreated)
                continue;
            switch(additional->type) {
                case CFGFieldType::STRING:
                    importStruct.additionalData.push_back(import->GetItemByIndex<std::string>(i)->GetValue());
                    break;
                case CFGFieldType::NUMBER:
                    if (import->GetItemByIndex(i)->HasType<int>())
                        importStruct.additionalData.push_back(import->GetItemByIndex<int>(i)->GetValue());
                    else
                        importStruct.additionalData.push_back(import->GetItemByIndex<float>(i)->GetValue());
                    break;
                case CFGFieldType::INTEGER:
                    importStruct.additionalData.push_back(import->GetItemByIndex<int>(i)->GetValue());
                    break;
                case CFGFieldType::FLOAT:
                    importStruct.additionalData.push_back(import->GetItemByIndex<float>(i)->GetValue());
                    break;
            }
        }
        imports.imports.push_back(importStruct);
    }
    return imports;
}

Resources::Imports<Resources::ShaderImport> Resources::ListShaderImports(const CFG::CFGField<std::vector<CFG::ICFGField*>>* obj) {
    using namespace CFG;

    Imports<ShaderImport> imports = Imports<ShaderImport>(ResourceType::SHADER);
    if (obj == nullptr)
        return imports;
    if (obj->name.has_value())
        imports.parentPath = Paths::Path(Paths::RESOURCES_DIR, obj->name.value());
    const std::vector<ICFGField*>& items = obj->GetItems();
    for (const ICFGField* v : items) {
        if (v->type != CFGFieldType::STRUCT) {
            spdlog::warn("'{}' cannot be parsed as a list of shader imports!", obj->name.value_or(""));
            return imports;
        }
    }
    for (const ICFGField* v : items) {
        const CFGObject* import = static_cast<const CFGObject*>(v);
        if (import == nullptr)
            continue;
        
        const auto* vertField = import->GetItemByIndex<std::string>(0);
        const auto* fragField = import->GetItemByIndex<std::string>(1);
        const auto* geomField = import->GetItemByIndex<std::string>(2);
        if (vertField == nullptr || fragField == nullptr || geomField == nullptr)
            continue;
        Resources::ShaderImport importStruct;
        importStruct.id = import->name.value_or("");
        importStruct.vertexPath = vertField->GetValue();
        importStruct.fragmentPath = fragField->GetValue();
        importStruct.geometryPath = geomField->GetValue();

        imports.imports.push_back(importStruct);
    }
    return imports;
}

void ResourceManager::LoadImports(const CFG::CFGObject* root) {
    auto resourceLoadEvent = [&](const std::string& resource) {
        eventHandler.Dispatch<const std::string&>(ResourceLoadEvent::ON_IMPORT_LOAD, resource);
    };
    std::unordered_map<std::string, Resources::ResourceType> cfgTypes = {
        { "[Texture]", Resources::ResourceType::TEXTURE },
        { "[Model]", Resources::ResourceType::MODEL },
        { "[Font]", Resources::ResourceType::FONT },
        { "[Stage]", Resources::ResourceType::STAGE },
        { "[Shader]", Resources::ResourceType::SHADER }
    };
    std::unordered_map<Resources::ResourceType, std::function<void(const CFG::CFGObject*, Resources::ResourceType)>> loaders = {
        {
            Resources::ResourceType::TEXTURE,
            [&](const CFG::CFGObject* obj, Resources::ResourceType t) {
                EventID textureLoadEvent = textureManager.onResourceLoad.Subscribe(resourceLoadEvent);
                textureManager.LoadImports(Resources::ListImports(obj, t));
                textureManager.onResourceLoad.Unsubscribe(textureLoadEvent);
            }
        },
        {
            Resources::ResourceType::MODEL,
            [&](const CFG::CFGObject* obj, Resources::ResourceType t) {
                EventID modelLoadEvent = modelManager.onResourceLoad.Subscribe(resourceLoadEvent);
                modelManager.LoadImports(Resources::ListImports(obj, t));
                modelManager.onResourceLoad.Unsubscribe(modelLoadEvent);
            }
        },
        {
            Resources::ResourceType::FONT,
            [&](const CFG::CFGObject* obj, Resources::ResourceType t) {
                EventID fontLoadEvent = fontManager.onResourceLoad.Subscribe(resourceLoadEvent);
                fontManager.LoadImports(Resources::ListImports(obj, t));
                fontManager.onResourceLoad.Unsubscribe(fontLoadEvent);
            }
        },
        {
            Resources::ResourceType::STAGE,
            [&](const CFG::CFGObject* obj, Resources::ResourceType t) {
                EventID stageLoadEvent = stageManager.onResourceLoad.Subscribe(resourceLoadEvent);
                stageManager.LoadImports(Resources::ListImports(obj, t));
                stageManager.onResourceLoad.Unsubscribe(stageLoadEvent);
            }
        },
        {
            Resources::ResourceType::SHADER,
            [&](const CFG::CFGObject* obj, Resources::ResourceType) {
                EventID shaderLoadEvent = shaderManager.onResourceLoad.Subscribe(resourceLoadEvent);
                shaderManager.LoadImports(Resources::ListShaderImports(obj));
                shaderManager.onResourceLoad.Unsubscribe(shaderLoadEvent);
            }
        }
    };
    
    size_t importCount = 0;
    std::unordered_map<Resources::ResourceType, std::vector<const CFG::CFGObject*>> imports;
    for (const auto& importList : root->GetItems()) {
        if (importList->type != CFG::CFGFieldType::ARRAY)
            continue;
        auto t = cfgTypes.find(importList->typeAnnotation);
        if (t == cfgTypes.end())
            continue;
        const CFG::CFGObject* importListObj = static_cast<const CFG::CFGObject*>(importList);
        imports[t->second].push_back(static_cast<const CFG::CFGObject*>(importListObj));
        importCount += importListObj->GetItems().size();
    }

    auto loadResources = [&](Resources::ResourceType t) {
        for (const auto& list : imports[t]) {
            loaders[t](list, t);
        }
    };

    eventHandler.Dispatch(ResourceLoadEvent::IMPORTS_INDEXED, importCount);

    loadResources(Resources::ResourceType::TEXTURE);
    loadResources(Resources::ResourceType::SHADER);

    materialsFile.DeserializeFile(Paths::MATERIALS_PATH.string());
    materialsFile.Register(Game::GetGameInstanceBase()->GetRenderer().GetMaterials());

    objectsFile.DeserializeFile(Paths::OBJECTS_PATH.string());

    loadResources(Resources::ResourceType::MODEL);
    loadResources(Resources::ResourceType::FONT);

    Serializer::BlueprintSerializer blueprints;
    blueprints.DeserializeFile(Paths::BLUEPRINTS_PATH.string());
    stageManager.UseBlueprints(&blueprints);
    loadResources(Resources::ResourceType::STAGE);
    stageManager.UseBlueprints(nullptr);
}

void ResourceManager::UnloadAll() {
    // TODO: clear all resourcemanagers
}