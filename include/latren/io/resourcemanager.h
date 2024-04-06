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
#include "serializablestruct.h"
#include "configs.h"
#include "files/materials.h"
#include "files/objects.h"
#include "files/blueprints.h"
#include "files/cfg.h"
#include <latren/gamewindow.h>
#include <latren/stage.h>
#include <latren/graphics/shader.h>
#include <latren/graphics/texture.h>
#include <latren/graphics/model.h>
#include <latren/ui/text.h>

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
    class ResourceManager {
    private:
        struct ItemComp {
            bool operator() (const std::string& l, const std::string& r) const {
                #ifdef strcasecmp
                    return strcasecmp(l.c_str(), r.c_str()) < 0;
                #else
                    return _stricmp(l.c_str(), r.c_str()) < 0;
                #endif
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
        ResourceManager(const std::fs::path& p, const std::string& t = "resource") : defaultPath_(p), typeStr_(t) {
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

    class TextureManager : public ResourceManager<Texture::TextureID> {
    protected:
        LATREN_API std::optional<Texture::TextureID> LoadResource(const std::fs::path&) override;
    public:
        LATREN_API TextureManager();
    };

    class ShaderManager : public ResourceManager<GLuint> {
    protected:
        LATREN_API std::optional<GLuint> LoadResource(const std::fs::path&) override;
        LATREN_API void LoadShader(GLuint, const std::string&, Shaders::ShaderType);
        LATREN_API void LoadShader(const std::string&, const std::string&, const std::string&, const std::string& = "");
        LATREN_API void LoadStandardShader(Shaders::ShaderID, const std::string&, Shaders::ShaderType);
        LATREN_API void LoadStandardShader(Shaders::ShaderID, const std::string&, const std::string&, const std::string& = "");
    public:
        LATREN_API void Load(const Resources::ShaderImport&);
        LATREN_API void LoadImports(const Imports<ShaderImport>&);
        LATREN_API ShaderManager();
        LATREN_API virtual void LoadStandardShaders();
        LATREN_API GLuint& Get(Shaders::ShaderID);
        GLuint& Get(const std::string& s) override { return ResourceManager::Get(s); }
    };

    #define BASE_FONT_SIZE 48
    class FontManager : public ResourceManager<UI::Text::Font> {
    protected:
        LATREN_API std::optional<UI::Text::Font> LoadResource(const std::fs::path&) override;
        glm::ivec2 fontSize_ = { 0, BASE_FONT_SIZE };
    public:
        LATREN_API FontManager();
        LATREN_API void SetFontSize(const glm::ivec2&);
        LATREN_API void SetFontSize(int);
    };

    class ModelManager : public ResourceManager<Model> {
    protected:
        LATREN_API std::optional<Model> LoadResource(const std::fs::path&) override;
    public:
        LATREN_API ModelManager();
    };

    class StageManager : public ResourceManager<Stage> {
    protected:
        LATREN_API std::optional<Stage> LoadResource(const std::fs::path&) override;
        std::vector<std::string> loadedStages_;
        Serializer::BlueprintSerializer* blueprints_ = nullptr;
    public:
        LATREN_API StageManager();
        LATREN_API const std::vector<std::string>& GetLoadedStages();
        LATREN_API bool LoadStage(const std::string&);
        LATREN_API bool UnloadStage(const std::string&);
        LATREN_API void UnloadAllStages();
        LATREN_API void UseBlueprints(Serializer::BlueprintSerializer*);
    };

    LATREN_API void SaveConfig(const std::fs::path&, const SerializableStruct&);
    LATREN_API void LoadConfig(const std::fs::path&, SerializableStruct&);

    LATREN_API Imports<Import> ListImports(const CFG::CFGField<std::vector<CFG::ICFGField*>>*, ResourceType);
    LATREN_API Imports<ShaderImport> ListShaderImports(const CFG::CFGField<std::vector<CFG::ICFGField*>>*);

    class ImportsFileTemplate : public CFG::CFGFileTemplateFactory {
        CFG::CFGCustomTypes DefineCustomTypes() const override {
            using namespace CFG;
            return {
                { "Font", { CFG_STRUCT(CFG_REQUIRE(CFGFieldType::STRING), CFGFieldType::INTEGER) } },
                { "Model", { CFG_STRUCT(CFG_REQUIRE(CFGFieldType::STRING)) } },
                { "Shader", { CFG_STRUCT(CFG_REQUIRE(CFGFieldType::STRING), CFG_REQUIRE(CFGFieldType::STRING), CFGFieldType::STRING, CFGFieldType::STRING) } },
                { "Stage", { CFG_STRUCT(CFG_REQUIRE(CFGFieldType::STRING)) } },
                { "Texture", { CFG_STRUCT(CFG_REQUIRE(CFGFieldType::STRING)) } }
            };
        }
    };
};

class ResourceManager {
public:
    enum class ResourceLoadEvent {
        IMPORTS_INDEXED, // eventargs: size_t totalImports
        ON_IMPORT_LOAD // eventargs: string id
    };

    Serializer::MaterialSerializer materialsFile;
    Serializer::ObjectSerializer objectsFile;

    Config::VideoSettings videoSettings;

    // LATREN_API void OverrideImportPath(Resources::ResourceType, const std::string&);
    // LATREN_API void OverrideImportCFGField(Resources::ResourceType, const std::string&);

    Resources::TextureManager textureManager;
    Resources::ShaderManager shaderManager;
    Resources::FontManager fontManager;
    Resources::ModelManager modelManager;
    Resources::StageManager stageManager;

    VariantEventHandler<ResourceLoadEvent,
        void(size_t),
        void(const std::string&)> eventHandler;

    LATREN_API void LoadImports(const CFG::CFGObject*);
    LATREN_API void LoadConfigs();
    LATREN_API void UnloadAll();
};