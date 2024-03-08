#include <latren/io/resourcemanager.h>
#include <latren/io/files/cfg.h>
#include <latren/io/paths.h>
#include <latren/io/serializablestruct.h>
#include <latren/game.h>

#include <fstream>

typedef std::string ResourceName;

namespace Resources {
    TextureManager::TextureManager() : ResourceManager<Texture::TextureID>(Paths::TEXTURES_DIR, ResourceName("texture")) { }
    ShaderManager::ShaderManager() : ResourceManager<GLuint>(Paths::SHADER_DIR) { }
    FontManager::FontManager() : ResourceManager<UI::Text::Font>(Paths::FONTS_DIR, ResourceName("font")) { }
    ModelManager::ModelManager() : ResourceManager<Model>(Paths::MODELS_DIR, ResourceName("model")) { }
    StageManager::StageManager() : ResourceManager<Stage>(Paths::STAGES_DIR, ResourceName("stage")) { }
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

std::vector<Resources::Import> Resources::ListImports(const CFG::CFGField<std::vector<CFG::ICFGField*>>* obj) {
    using namespace CFG;

    if (obj == nullptr)
        return { };
    std::vector<Import> imports;
    const std::vector<ICFGField*>& items = obj->GetItems();
    for (const ICFGField* v : items) {
        if (v->type != CFGFieldType::STRUCT) {
            spdlog::warn("'{}' cannot be parsed as a list of imports!", obj->name);
            return { };
        }
        const CFGObject* import = static_cast<const CFGObject*>(v);
        if (import == nullptr)
            continue;
        const auto* pathField = import->GetItemByIndex<std::string>(0);
        if (pathField == nullptr)
            continue;
        Resources::Import importStruct;
        importStruct.id = import->name;
        importStruct.path = pathField->GetValue();
        for (int i = 2; i < import->GetItems().size(); i++) {
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
        imports.push_back(importStruct);
    }
    return imports;
}

std::vector<Resources::ShaderImport> Resources::ListShaderImports(const CFG::CFGField<std::vector<CFG::ICFGField*>>* obj) {
    using namespace CFG;

    if (obj == nullptr)
        return { };
    std::vector<ShaderImport> imports;
    const std::vector<ICFGField*>& items = obj->GetItems();
    for (const ICFGField* v : items) {
        if (v->type != CFGFieldType::STRUCT) {
            spdlog::warn("'{}' cannot be parsed as a list of shader imports!", obj->name);
            return { };
        }
        const CFGObject* import = static_cast<const CFGObject*>(v);
        if (import == nullptr)
            continue;
        
        const auto* vertField = import->GetItemByIndex<std::string>(0);
        const auto* fragField = import->GetItemByIndex<std::string>(1);
        const auto* geomField = import->GetItemByIndex<std::string>(2);
        if (vertField == nullptr || fragField == nullptr || geomField == nullptr)
            continue;
        Resources::ShaderImport importStruct;
        importStruct.id = import->name;
        importStruct.vertexPath = vertField->GetValue();
        importStruct.fragmentPath = fragField->GetValue();
        importStruct.geometryPath = geomField->GetValue();

        imports.push_back(importStruct);
    }
    return imports;
}

void ResourceManager::LoadImports(const CFG::CFGObject* imports) {
    std::vector<Resources::Import> textures = Resources::ListImports(imports->GetObjectByName("textures"));
    std::vector<Resources::Import> models = Resources::ListImports(imports->GetObjectByName("models"));
    std::vector<Resources::Import> fonts = Resources::ListImports(imports->GetObjectByName("fonts"));
    std::vector<Resources::Import> stages = Resources::ListImports(imports->GetObjectByName("stages"));
    std::vector<Resources::ShaderImport> shaders = Resources::ListShaderImports(imports->GetObjectByName("shaders"));
    size_t importCount =    textures.size() +
                            models.size() +
                            fonts.size() +
                            stages.size() +
                            shaders.size();

    eventHandler.Dispatch(ResourceLoadEvent::IMPORTS_INDEXED, importCount);

    auto resourceLoadEvent = [&](const std::string& resource) {
        eventHandler.Dispatch<const std::string&>(ResourceLoadEvent::ON_IMPORT_LOAD, resource);
    };

    EventID textureLoadEvent = textureManager.onResourceLoad.Subscribe(resourceLoadEvent);
    textureManager.LoadImports(textures);
    textureManager.onResourceLoad.Unsubscribe(textureLoadEvent);

    EventID shaderLoadEvent = shaderManager.onResourceLoad.Subscribe(resourceLoadEvent);
    shaderManager.LoadImports(shaders);
    shaderManager.onResourceLoad.Unsubscribe(shaderLoadEvent);

    materialsFile.DeserializeFile(Paths::MATERIALS_PATH.string());
    materialsFile.Register(Game::GetGameInstanceBase()->GetRenderer().GetMaterials());

    objectsFile.DeserializeFile(Paths::OBJECTS_PATH.string());

    EventID modelLoadEvent = modelManager.onResourceLoad.Subscribe(resourceLoadEvent);
    modelManager.LoadImports(models);
    modelManager.onResourceLoad.Unsubscribe(modelLoadEvent);

    EventID fontLoadEvent = fontManager.onResourceLoad.Subscribe(resourceLoadEvent);
    fontManager.LoadImports(fonts);
    fontManager.onResourceLoad.Unsubscribe(fontLoadEvent);

    Serializer::BlueprintSerializer blueprints;
    blueprints.DeserializeFile(Paths::BLUEPRINTS_PATH.string());
    stageManager.UseBlueprints(&blueprints);
    
    EventID stageLoadEvent = stageManager.onResourceLoad.Subscribe(resourceLoadEvent);
    stageManager.LoadImports(stages);
    stageManager.onResourceLoad.Unsubscribe(stageLoadEvent);
    stageManager.UseBlueprints(nullptr);
}

void ResourceManager::UnloadAll() {
    // TODO: clear all resourcemanagers
}