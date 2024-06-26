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
#include "fs.h"
#include "configs.h"
#include "files/materials.h"
#include "files/objects.h"
#include "files/blueprints.h"
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
    LATREN_API const std::fs::path& GetDefaultPath(ResourceType);
    template <typename ImportType = Import>
    struct Imports {
        ResourceType resourceType;
        std::fs::path parentPath;
        std::vector<ImportType> imports;

        Imports(ResourceType t, const std::fs::path& p, const std::vector<ImportType>& i = { }) : resourceType(t), parentPath(p), imports(i) { }
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
        std::fs::path path_;
        const std::fs::path defaultPath_;
        virtual std::optional<T> LoadResource(const std::fs::path&) = 0;
        void SetItemID(const std::string& id) { itemID_ = id; }
        void SetAdditionalData(const AdditionalImportData& data) { additionalData_ = data; }
        const std::string& GetItemID() { return itemID_; }
        const AdditionalImportData& GetAdditionalData() { return additionalData_; }
    public:
        SingleEventHandler<const std::string&> onResourceLoad;
        virtual ~ResourceTypeManager() = default;
        ResourceTypeManager(const std::fs::path& p, const std::string& t = "resource") : defaultPath_(p), typeStr_(t) {
            path_ = defaultPath_;
        }
        virtual void Load(const std::fs::path& p) {
            onResourceLoad.Dispatch(itemID_);
            if (items_.find(itemID_) != items_.end())
                return;
            std::string fileName = std::fs::proximate(p, path_.parent_path()).generic_string();
            spdlog::info("Loading {} '{}'", typeStr_, fileName);
            std::optional<T> resource = LoadResource(std::fs::absolute(p));
            spdlog::debug("  (id: {})", itemID_);
            if (resource.has_value())
                items_[itemID_] = resource.value();
            else
                spdlog::info("Failed loading {} '{}'", typeStr_, fileName);
        }
        virtual void Load(const Import& import) {
            std::fs::path importPath = import.path.ParsePath(path_);
            if (import.id.empty())
                SetItemID(std::fs::proximate(importPath, path_).generic_string());
            else
                SetItemID(import.id);
            SetAdditionalData(import.additionalData);
            Load(importPath);
        }
        void SetPath(const std::fs::path& p) {
            path_ = p;
        }
        void RestoreDefaultPath() {
            path_ = defaultPath_;
        }
        virtual void LoadImports() {
            for (const auto& f : std::fs::directory_iterator(path_))
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
        std::optional<Texture::TextureID> LoadResource(const std::fs::path&) override;
    public:
        TextureManager();
    };

    class LATREN_API ShaderManager : public ResourceTypeManager<GLuint> {
    protected:
        std::optional<GLuint> LoadResource(const std::fs::path&) override;
        void LoadShader(GLuint, const std::string&, Shaders::ShaderType);
        void LoadShader(const std::string&, const std::string&, const std::string&, const std::string& = "");
        void LoadStandardShader(Shaders::ShaderID, const std::string&, Shaders::ShaderType);
        void LoadStandardShader(Shaders::ShaderID, const std::string&, const std::string&, const std::string& = "");
    public:
        void Load(const Resources::ShaderImport&);
        void LoadImports(const Imports<ShaderImport>&);
        ShaderManager();
        virtual void LoadStandardShaders();
        GLuint& Get(Shaders::ShaderID);
        GLuint& Get(const std::string& s) override { return ResourceTypeManager::Get(s); }
    };

    #define BASE_FONT_SIZE 48
    class LATREN_API FontManager : public ResourceTypeManager<UI::Text::Font> {
    protected:
        glm::ivec2 fontSize_ = { 0, BASE_FONT_SIZE };
        std::optional<UI::Text::Font> LoadResource(const std::fs::path&) override;
    public:
        FontManager();
        void SetFontSize(const glm::ivec2&);
        void SetFontSize(int);
    };

    class LATREN_API ModelManager : public ResourceTypeManager<Model> {
    protected:
        std::optional<Model> LoadResource(const std::fs::path&) override;
    public:
        ModelManager();
    };

    class LATREN_API AudioManager : public ResourceTypeManager<AudioBufferHandle> {
    protected:
        std::optional<AudioBufferHandle> LoadResource(const std::fs::path&) override;
    public:
        AudioManager();
    };

    class LATREN_API StageManager : public ResourceTypeManager<Stage> {
    protected:
        std::optional<Stage> LoadResource(const std::fs::path&) override;
        std::vector<std::string> loadedStages_;
        Serializer::BlueprintSerializer* blueprints_ = nullptr;
    public:
        StageManager();
        const std::vector<std::string>& GetLoadedStages();
        bool LoadStage(const std::string&);
        bool UnloadStage(const std::string&);
        void UnloadAllStages();
        void UseBlueprints(Serializer::BlueprintSerializer*);
    };

    class LATREN_API TextFileManager : public ResourceTypeManager<std::string> {
    protected:
        std::optional<std::string> LoadResource(const std::fs::path&) override;
    public:
        TextFileManager();
    };

    struct BinaryFile {
        size_t size;
        uint8_t* buffer;

        virtual ~BinaryFile() {
            delete[] buffer;
        }
    };
    class LATREN_API BinaryFileManager : public ResourceTypeManager<BinaryFile> {
    protected:
        std::optional<BinaryFile> LoadResource(const std::fs::path&) override;
    public:
        BinaryFileManager();
    };
    class LATREN_API JSONFileManager : public ResourceTypeManager<nlohmann::json> {
    protected:
        std::optional<nlohmann::json> LoadResource(const std::fs::path&) override;
    public:
        JSONFileManager();
    };
    class LATREN_API CFGFileManager : public ResourceTypeManager<CFG::CFGObject*> {
    protected:
        std::optional<CFG::CFGObject*> LoadResource(const std::fs::path&) override;
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
        IMPORTS_INDEXED, // eventargs: size_t totalImports
        ON_IMPORT_LOAD // eventargs: string id
    };
};

class LATREN_API ResourceManager {
public:
    Serializer::MaterialSerializer materialsFile;
    Serializer::ObjectSerializer objectsFile;

    Config::VideoSettings videoSettings;

    // LATREN_API void OverrideImportPath(Resources::ResourceType, const std::string&);
    // LATREN_API void OverrideImportCFGField(Resources::ResourceType, const std::string&);

    Resources::TextureManager textureManager;
    Resources::ShaderManager shaderManager;
    Resources::FontManager fontManager;
    Resources::ModelManager modelManager;
    Resources::AudioManager audioManager;
    Resources::StageManager stageManager;

    struct {
        Resources::TextFileManager textFileManager;
        Resources::BinaryFileManager binaryFileManager;
        Resources::JSONFileManager jsonFileManager;
        Resources::CFGFileManager cfgFileManager;
    } dataFiles;

    VariantEventHandler<Resources::ResourceLoadEvent,
        void(size_t),
        void(const std::string&)> eventHandler;

    void LoadImports(const CFG::CFGObject*);
    void LoadConfigs();
    void UnloadAll();
};