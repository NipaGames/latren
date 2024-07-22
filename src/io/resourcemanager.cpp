#include <latren/io/resourcemanager.h>
#include <latren/io/files/cfg.h>
#include <latren/io/paths.h>
#include <latren/io/serializablestruct.h>
#include <latren/systems.h>
#include <latren/graphics/renderer.h>

#include <fstream>

typedef std::string ResourceName;

const ResourcePath& Resources::GetDefaultPath(Resources::ResourceType t) {
    return Paths::RESOURCE_DIRS.at(t);
}
namespace Resources {
    TextureManager::TextureManager() : ResourceTypeManager<Texture::TextureID>(GetDefaultPath(ResourceType::TEXTURE), ResourceName("texture")) { }
    ShaderManager::ShaderManager() : ResourceTypeManager<GLuint>(GetDefaultPath(ResourceType::SHADER)) { }
    FontManager::FontManager() : ResourceTypeManager<UI::Text::Font>(GetDefaultPath(ResourceType::FONT), ResourceName("font")) { }
    ModelManager::ModelManager() : ResourceTypeManager<Model>(GetDefaultPath(ResourceType::MODEL), ResourceName("model")) { }
    StageManager::StageManager() : ResourceTypeManager<Stage>(GetDefaultPath(ResourceType::STAGE), ResourceName("stage")) { }
    AudioManager::AudioManager() : ResourceTypeManager<AudioBufferHandle>(GetDefaultPath(ResourceType::AUDIO), ResourceName("audio file")) { }

    TextFileManager::TextFileManager() : ResourceTypeManager<std::string>(GetDefaultPath(ResourceType::TEXT), ResourceName("text file")) { }
    BinaryFileManager::BinaryFileManager() : ResourceTypeManager<BinaryFile>(GetDefaultPath(ResourceType::BINARY), ResourceName("binary file")) { }
    JSONFileManager::JSONFileManager() : ResourceTypeManager<nlohmann::json>(GetDefaultPath(ResourceType::JSON), ResourceName("JSON file")) { }
    CFGFileManager::CFGFileManager() : ResourceTypeManager<CFG::CFGObject*>(GetDefaultPath(ResourceType::CFG), ResourceName("CFG file")) { }
};

CFG::CFGCustomTypes Resources::ImportsFileTemplate::DefineCustomTypes() const {
    using namespace CFG;
    return {
        { "Font", { CFG_STRUCT(CFG_REQUIRE(CFGFieldType::STRING), CFGFieldType::INTEGER) } },
        { "Model", { CFG_STRUCT(CFG_REQUIRE(CFGFieldType::STRING)) } },
        { "Shader", { CFG_STRUCT(CFG_REQUIRE(CFGFieldType::STRING), CFG_REQUIRE(CFGFieldType::STRING), CFGFieldType::STRING, CFGFieldType::STRING) } },
        { "Stage", { CFG_STRUCT(CFG_REQUIRE(CFGFieldType::STRING)) } },
        { "Texture", { CFG_STRUCT(CFG_REQUIRE(CFGFieldType::STRING)) } },
        { "Audio", { CFG_STRUCT(CFG_REQUIRE(CFGFieldType::STRING)) } },

        { "Text", { CFG_STRUCT(CFG_REQUIRE(CFGFieldType::STRING)) } },
        { "Binary", { CFG_STRUCT(CFG_REQUIRE(CFGFieldType::STRING)) } },
        { "JSON", { CFG_STRUCT(CFG_REQUIRE(CFGFieldType::STRING)) } },
        { "CFG", { CFG_STRUCT(CFG_REQUIRE(CFGFieldType::STRING)) } }
    };
}

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
    std::fs::path usrDir = ResourcePath("${usr}").GetParsedPath();
    std::fs::path saveDataDir = ResourcePath("${savedata}").GetParsedPath();
    if (!std::fs::is_directory(usrDir) || !std::fs::exists(usrDir))
        std::fs::create_directory(usrDir);
    if (!std::fs::is_directory(saveDataDir) || !std::fs::exists(saveDataDir))
        std::fs::create_directory(saveDataDir);
    
    Resources::LoadConfig(ResourcePath("${video.cfg}").GetParsedPath(), videoSettings);
}

Resources::Imports<Resources::Import> Resources::ListImports(const CFG::CFGField<std::vector<CFG::ICFGField*>>* obj, ResourceType t) {
    using namespace CFG;
    Imports<Import> imports = Imports<Import>(t);

    if (obj == nullptr)
        return imports;
    if (obj->name.has_value())
        imports.parentPath = ResourcePath("${res}", obj->name.value());
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
        imports.parentPath = ResourcePath("${res}", obj->name.value());
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

template <typename T>
std::function<void(const CFG::CFGObject*, Resources::ResourceType)> CreateLoader(T& mgr, VariantEventHandler<Resources::ResourceLoadEvent, void(std::size_t), void(const std::string&)>& eventHandler) {
    using namespace Resources;
    
    return [&](const CFG::CFGObject* obj, ResourceType t) {
        EventID event = mgr.onResourceLoad.Subscribe([&](const std::string& resource) {
            eventHandler.Dispatch<const std::string&>(ResourceLoadEvent::ON_IMPORT_LOAD, resource);
        });
        mgr.LoadImports(ListImports(obj, t));
        mgr.onResourceLoad.Unsubscribe(event);
    };
}

void ResourceManager::LoadImports(const CFG::CFGObject* root) {
    using namespace Resources;

    std::unordered_map<ResourceType, std::function<void(const CFG::CFGObject*, ResourceType)>> loaders = {
        { ResourceType::TEXTURE, CreateLoader(textureManager, eventHandler) },
        { ResourceType::MODEL, CreateLoader(modelManager, eventHandler) },
        { ResourceType::FONT, CreateLoader(fontManager, eventHandler) },
        { ResourceType::STAGE, CreateLoader(stageManager, eventHandler) },
        { ResourceType::SHADER,
            [&](const CFG::CFGObject* obj, ResourceType) {
                EventID event = shaderManager.onResourceLoad.Subscribe([&](const std::string& resource) {
                    eventHandler.Dispatch<const std::string&>(ResourceLoadEvent::ON_IMPORT_LOAD, resource);
                });
                shaderManager.LoadImports(ListShaderImports(obj));
                shaderManager.onResourceLoad.Unsubscribe(event);
            }
        },
        { ResourceType::AUDIO, CreateLoader(audioManager, eventHandler) },

        { ResourceType::TEXT, CreateLoader(dataFiles.textFileManager, eventHandler) },
        { ResourceType::BINARY, CreateLoader(dataFiles.binaryFileManager, eventHandler) },
        { ResourceType::JSON, CreateLoader(dataFiles.jsonFileManager, eventHandler) },
        { ResourceType::CFG, CreateLoader(dataFiles.cfgFileManager, eventHandler) }
    };
    std::unordered_map<std::string, ResourceType> cfgTypes = {
        { "[Texture]", ResourceType::TEXTURE },
        { "[Model]", ResourceType::MODEL },
        { "[Font]", ResourceType::FONT },
        { "[Stage]", ResourceType::STAGE },
        { "[Shader]", ResourceType::SHADER },
        { "[Audio]", ResourceType::AUDIO },

        { "[Text]", ResourceType::TEXT },
        { "[Binary]", ResourceType::BINARY },
        { "[JSON]", ResourceType::JSON },
        { "[CFG]", ResourceType::CFG }
    };
    
    std::size_t importCount = 0;
    std::unordered_map<ResourceType, std::vector<const CFG::CFGObject*>> imports;
    for (const auto& importList : root->GetItems()) {
        if (importList->type != CFG::CFGFieldType::ARRAY)
            continue;
        auto t = cfgTypes.find(importList->typeAnnotation);
        if (t == cfgTypes.end())
            continue;
        if ((resourceTypesToLoad & t->second) == 0)
            continue;
        const CFG::CFGObject* importListObj = static_cast<const CFG::CFGObject*>(importList);
        imports[t->second].push_back(static_cast<const CFG::CFGObject*>(importListObj));
        importCount += importListObj->GetItems().size();
    }

    auto loadImports = [&](ResourceType t) {
        for (const auto& list : imports[t]) {
            loaders[t](list, t);
        }
    };

    eventHandler.Dispatch(ResourceLoadEvent::IMPORTS_INDEXED, importCount);

    loadImports(ResourceType::TEXTURE);
    loadImports(ResourceType::SHADER);

    materialsFile.DeserializeFile("${materials.json}"_resp);
    materialsFile.Register(Systems::GetRenderer().GetMaterials());

    objectsFile.DeserializeFile("${objects.json}"_resp);

    loadImports(ResourceType::MODEL);
    loadImports(ResourceType::FONT);
    loadImports(ResourceType::AUDIO);
    
    loadImports(ResourceType::TEXT);
    loadImports(ResourceType::BINARY);
    loadImports(ResourceType::JSON);
    loadImports(ResourceType::CFG);

    if ((resourceTypesToLoad & ResourceType::STAGE) != 0) {
        Serializer::BlueprintSerializer blueprints;
        blueprints.DeserializeFile("${blueprints.json}"_resp);
        stageManager.UseBlueprints(&blueprints);
        loadImports(ResourceType::STAGE);
        stageManager.UseBlueprints(nullptr);
    }
}

void ResourceManager::UnloadAll() {
    // TODO: clear all resourcemanagers
}