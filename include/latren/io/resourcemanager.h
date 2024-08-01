#pragma once

#include <fstream>
#include <string>
#include <cstring>
#include <unordered_map>
#include <map>
#include <filesystem>
#include <optional>
#include <variant>
#include <spdlog/spdlog.h>

#include "resourcepath.h"
#include "resourcetype.h"
#include "configs.h"
#include "files/materials.h"
#include "files/objects.h"
#include "files/blueprints.h"
#include "files/stage.h"
#include "files/cfg.h"
#include <latren/stage.h>
#include <latren/graphics/shader.h>
#include <latren/graphics/texture.h>
#include <latren/graphics/model.h>
#include <latren/ui/text.h>
#include <latren/audio/alhandle.h>
#include <latren/event/eventhandler.h>

struct SerializableStruct;

namespace Resources {
    using AdditionalImportData = std::vector<std::variant<std::string, float, int>>;
    struct Import {
        ResourcePath path;
        std::string id;
        AdditionalImportData additionalData;
    };
    LATREN_API const ResourcePath& GetDefaultPath(ResourceType);
    template <typename ImportType = Import>
    struct Imports {
        ResourceType resourceType;
        ResourcePath parentPath;
        std::vector<ImportType> imports;

        Imports(ResourceType t, const ResourcePath& p, const std::vector<ImportType>& i = { }) : resourceType(t), parentPath(p), imports(i) { }
        Imports(ResourceType t, const std::vector<ImportType>& i = { }) : resourceType(t), parentPath(GetDefaultPath(t)), imports(i) { }
    };
    struct ShaderImport {
        std::string id;
        ResourcePath vertexPath;
        ResourcePath fragmentPath;
        ResourcePath geometryPath;
    };
    template <typename T>
    class ResourceTypeManager {
    private:
        struct ItemComp {
            bool operator() (const std::string& l, const std::string& r) const {
                return std::lexicographical_compare(l.begin(), l.end(), r.begin(), r.end(), [](char lc, char rc) {
                    return std::tolower(lc) < std::tolower(rc);
                });
            }
        };
        std::string itemID_;
        std::string typeStr_;
        AdditionalImportData additionalData_;
    protected:
        std::map<std::string, T, ItemComp> items_;
        ResourcePath path_;
        const ResourcePath defaultPath_;
        virtual std::optional<T> LoadResource(const ResourcePath&) = 0;
        void SetItemID(const std::string& id) { itemID_ = id; }
        void SetAdditionalData(const AdditionalImportData& data) { additionalData_ = data; }
        const std::string& GetItemID() { return itemID_; }
        ResourcePath MakeImportPath(const ResourcePath& p) {
            const std::string& rawPath = p.GetUnparsedPathStr();
            if (rawPath.length() >= 2 && rawPath.at(1) == '!') {
                // just an escape
                if (rawPath.at(0) == '\\')
                    return ResourcePath(path_, rawPath.substr(1, rawPath.length() - 1));
                // starts with "!!"; override parent path
                else if (rawPath.at(0) == '!')
                    return rawPath.substr(2, rawPath.length() - 2);
            }
            return ResourcePath(path_, rawPath);
        }
        const AdditionalImportData& GetAdditionalData() { return additionalData_; }
    public:
        SingleEventHandler<const std::string&> onResourceLoad;
        virtual ~ResourceTypeManager() = default;
        ResourceTypeManager(const ResourcePath& p, const std::string& t = "resource") : defaultPath_(p), typeStr_(t) {
            path_ = defaultPath_;
        }
        virtual void Load(const ResourcePath& p) {
            onResourceLoad.Dispatch(itemID_);
            if (items_.find(itemID_) != items_.end())
                return;
            std::fs::path importPath = p.GetParsedPath();
            std::string fileName = std::fs::proximate(importPath, path_.GetParsedPath().parent_path()).generic_string();
            spdlog::info("Loading {} '{}'", typeStr_, fileName);
            std::optional<T> resource = LoadResource(std::fs::absolute(importPath));
            spdlog::debug("  (id: {})", itemID_);
            if (resource.has_value())
                items_[itemID_] = resource.value();
            else
                spdlog::info("Failed loading {} '{}'", typeStr_, fileName);
        }
        virtual void Load(const Import& import) {
            ResourcePath importPath = MakeImportPath(import.path);
            if (import.id.empty())
                SetItemID(std::fs::proximate(importPath.GetParsedPath(), path_.GetParsedPath()).generic_string());
            else
                SetItemID(import.id);
            SetAdditionalData(import.additionalData);
            Load(importPath);
        }
        void SetPath(const ResourcePath& p) {
            path_ = p;
        }
        void RestoreDefaultPath() {
            path_ = defaultPath_;
        }
        virtual void LoadImports() {
            for (const auto& f : std::fs::directory_iterator(path_.GetParsedPath()))
                Load(f.path());
        }
        void LoadImports(const Imports<Import>& imports) {
            SetPath(imports.parentPath);
            for (const auto& import : imports.imports)
                Load(import);
            RestoreDefaultPath();
        }
        virtual T& Get(const std::string& item) {
            return items_.at(item);
        }
        bool HasLoaded(const std::string& item) const {
            return items_.find(item) != items_.end();
        }
        T& operator[](const std::string& item) {
            if (!HasLoaded(item)) {
                T t;
                items_[item] = t;
            }
            return Get(item);
        }
        const std::map<std::string, T, ItemComp>& GetAll() {
            return items_;
        }
        void Set(const std::string& id, const T& val) {
            items_[id] = val;
        }
    };

    class LATREN_API TextureManager : public ResourceTypeManager<Texture::TextureID> {
    protected:
        virtual std::optional<Texture::TextureID> LoadResource(const ResourcePath&) override;
    public:
        TextureManager();
    };

    class LATREN_API ShaderManager : public ResourceTypeManager<GLuint> {
    protected:
        virtual std::optional<GLuint> LoadResource(const ResourcePath&) override;
        virtual void LoadShader(GLuint, const ResourcePath&, Shaders::ShaderType);
        virtual void LoadShader(const std::string&, const ResourcePath&, const ResourcePath&, const ResourcePath& = "");
        virtual void LoadStandardShader(Shaders::ShaderID, const ResourcePath&, Shaders::ShaderType);
        virtual void LoadStandardShader(Shaders::ShaderID, const ResourcePath&, const ResourcePath&, const ResourcePath& = "");
    public:
        ShaderManager();
        virtual void Load(const Resources::ShaderImport&);
        virtual void LoadImports(const Imports<ShaderImport>&);
        virtual void LoadStandardShaders();
        virtual GLuint& Get(Shaders::ShaderID);
        virtual GLuint& Get(const std::string& s) override { return ResourceTypeManager::Get(s); }
    };

    #define BASE_FONT_SIZE 48
    class LATREN_API FontManager : public ResourceTypeManager<UI::Text::Font> {
    protected:
        glm::ivec2 fontSize_ = { 0, BASE_FONT_SIZE };
        virtual std::optional<UI::Text::Font> LoadResource(const ResourcePath&) override;
    public:
        FontManager();
        virtual void SetFontSize(const glm::ivec2&);
        virtual void SetFontSize(int);
    };

    class LATREN_API ModelManager : public ResourceTypeManager<Model> {
    protected:
        virtual std::optional<Model> LoadResource(const ResourcePath&) override;
    public:
        ModelManager();
    };

    class LATREN_API AudioManager : public ResourceTypeManager<AudioBufferHandle> {
    protected:
        virtual std::optional<AudioBufferHandle> LoadResource(const ResourcePath&) override;
    public:
        AudioManager();
    };

    class LATREN_API StageManager : public ResourceTypeManager<Stage> {
    protected:
        std::vector<std::string> loadedStages_;
        std::unique_ptr<Serialization::StageSerializer> stageSerializer_;
        virtual std::optional<Stage> LoadResource(const ResourcePath&) override;
    public:
        StageManager();
        virtual const std::vector<std::string>& GetLoadedStages();
        virtual bool LoadStage(const std::string&);
        virtual bool UnloadStage(const std::string&);
        virtual void UnloadAllStages();
        virtual void UseBlueprints(Serialization::BlueprintSerializer*);
    };

    class LATREN_API TextFileManager : public ResourceTypeManager<std::string> {
    protected:
        virtual std::optional<std::string> LoadResource(const ResourcePath&) override;
    public:
        TextFileManager();
    };

    struct BinaryFile {
        std::size_t size;
        uint8_t* buffer;

        virtual ~BinaryFile() {
            delete[] buffer;
        }
    };
    class LATREN_API BinaryFileManager : public ResourceTypeManager<BinaryFile> {
    protected:
        virtual std::optional<BinaryFile> LoadResource(const ResourcePath&) override;
    public:
        BinaryFileManager();
    };
    class LATREN_API JSONFileManager : public ResourceTypeManager<nlohmann::json> {
    protected:
        virtual std::optional<nlohmann::json> LoadResource(const ResourcePath&) override;
    public:
        JSONFileManager();
    };
    class LATREN_API CFGFileManager : public ResourceTypeManager<CFG::CFGObject*> {
    protected:
        virtual std::optional<CFG::CFGObject*> LoadResource(const ResourcePath&) override;
    public:
        virtual ~CFGFileManager();
        CFGFileManager();
    };

    LATREN_API void SaveConfig(const std::fs::path&, const SerializableStruct&);
    LATREN_API void LoadConfig(const std::fs::path&, SerializableStruct&);

    LATREN_API Imports<Import> ListImports(const CFG::CFGField<std::vector<CFG::ICFGField*>>*, ResourceType);
    LATREN_API Imports<ShaderImport> ListShaderImports(const CFG::CFGField<std::vector<CFG::ICFGField*>>*);

    class ImportsFileTemplate : public CFG::CFGFileTemplateFactory {
        CFG::CFGCustomTypes DefineCustomTypes() const override;
    };

    enum class ResourceLoadEvent {
        IMPORTS_INDEXED, // eventargs: std::size_t totalImports
        ON_IMPORT_LOAD // eventargs: string id
    };
};

class IResourceManager {
public:
    VariantEventHandler<Resources::ResourceLoadEvent,
        void(std::size_t),
        void(const std::string&)> eventHandler;
    
    virtual void LoadImports(const CFG::CFGObject*) = 0;
    virtual void UnloadAll() = 0;
    
    virtual Serialization::MaterialSerializer* GetMaterialSerializer() = 0;
    virtual Serialization::ObjectSerializer* GetObjectSerializer() = 0;
    virtual Serialization::BlueprintSerializer* GetBlueprintSerializer() = 0;

    virtual Resources::TextureManager* GetTextureManager() = 0;
    virtual Resources::ShaderManager* GetShaderManager() = 0;
    virtual Resources::FontManager* GetFontManager() = 0;
    virtual Resources::ModelManager* GetModelManager() = 0;
    virtual Resources::AudioManager* GetAudioManager() = 0;
    virtual Resources::StageManager* GetStageManager() = 0;

    virtual Resources::TextFileManager* GetTextFileManager() = 0;
    virtual Resources::BinaryFileManager* GetBinaryFileManager() = 0;
    virtual Resources::JSONFileManager* GetJSONFileManager() = 0;
    virtual Resources::CFGFileManager* GetCFGFileManager() = 0;
};

// Disable (basic) resource loaders with unflagging them in the ctor
class LATREN_API ModularResourceManager : public IResourceManager {
// uh oh
typedef std::variant<
    Serialization::MaterialSerializer,
    Serialization::ObjectSerializer,
    Serialization::BlueprintSerializer,
    Resources::TextureManager,
    Resources::ShaderManager,
    Resources::FontManager,
    Resources::ModelManager,
    Resources::AudioManager,
    Resources::StageManager,
    Resources::TextFileManager,
    Resources::BinaryFileManager,
    Resources::JSONFileManager,
    Resources::CFGFileManager
> BasicResourceLoader;

private:
    std::unordered_map<Resources::ResourceType, BasicResourceLoader> basicResourceLoaders_;
protected:
    template <typename T>
    void AddBasicResourceLoaderIf(Resources::ResourceType check, Resources::ResourceType t) {
        if ((check & t) != 0)
            basicResourceLoaders_.insert({ t, T() });
    }
    template <typename T>
    T* GetBasicResourceLoader(Resources::ResourceType t) {
        if (basicResourceLoaders_.count(t) == 0)
            return nullptr;
        return std::get_if<T>(&basicResourceLoaders_.at(t));
    }
public:
    ModularResourceManager(Resources::ResourceType);

    virtual void LoadImports(const CFG::CFGObject*) override;
    virtual void UnloadAll() override;

    virtual Serialization::MaterialSerializer* GetMaterialSerializer() override;
    virtual Serialization::ObjectSerializer* GetObjectSerializer() override;
    virtual Serialization::BlueprintSerializer* GetBlueprintSerializer() override;

    virtual Resources::TextureManager* GetTextureManager() override;
    virtual Resources::ShaderManager* GetShaderManager() override;
    virtual Resources::FontManager* GetFontManager() override;
    virtual Resources::ModelManager* GetModelManager() override;
    virtual Resources::AudioManager* GetAudioManager() override;
    virtual Resources::StageManager* GetStageManager() override;
    virtual Resources::TextFileManager* GetTextFileManager() override;
    virtual Resources::BinaryFileManager* GetBinaryFileManager() override;
    virtual Resources::JSONFileManager* GetJSONFileManager() override;
    virtual Resources::CFGFileManager* GetCFGFileManager() override;
};

class GameResourceManager : public ModularResourceManager {
public:
    Config::VideoSettings videoSettings;

    GameResourceManager() : ModularResourceManager(Resources::ResourceType::ALL) { }
    virtual void LoadConfigs();
};