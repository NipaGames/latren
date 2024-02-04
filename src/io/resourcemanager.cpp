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
        config = {};
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

void ResourceManager::LoadImports(const CFG::CFGObject* imports) {
    std::vector<Resources::Import> textures = imports->ListImports("textures");
    std::vector<Resources::Import> models = imports->ListImports("models");
    std::vector<Resources::Import> fonts = imports->ListImports("fonts");
    std::vector<Resources::Import> stages = imports->ListImports("stages");
    std::vector<Resources::ShaderImport> shaders = imports->ListShaderImports("shaders");
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