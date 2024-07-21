#include <latren/graphics/shader.h>
#include <latren/systems.h>
#include <latren/io/paths.h>
#include <latren/io/resourcemanager.h>

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <spdlog/spdlog.h>

using namespace Shaders;

const std::string Shaders::EXT_VERT = ".vert";
const std::string Shaders::EXT_FRAG = ".frag";
const std::string Shaders::EXT_GEOM = ".geom";

std::string GetShaderInfoLog(GLuint shader) {
    int logLength;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0) {
		char* message = new char[logLength + 1];
		glGetShaderInfoLog(shader, logLength, nullptr, &message[0]);
		std::string result = &message[0];
        delete[] message;
        return result;
	}
    return "";
}

std::string GetProgramInfoLog(GLuint program) {
    int logLength;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0) {
		char* message = new char[logLength + 1];
		glGetProgramInfoLog(program, logLength, nullptr, &message[0]);
		std::string result = &message[0];
        delete[] message;
        return result;
	}
    return "";
}

void LoadShaderFromFile(GLuint shader, const ResourcePath& path) {
    std::string shaderData;
    std::ifstream shaderStream(path.GetParsedPathStr(), std::ios::in);
    if (shaderStream.is_open()) {
        std::stringstream ss;
        ss << shaderStream.rdbuf();
        shaderData = ss.str();
        shaderStream.close();
    }
    else {
        spdlog::error("Cannot read shader file!");
        return;
    }
    char const* shaderDataPtr = shaderData.c_str();
    glShaderSource(shader, 1, &shaderDataPtr, nullptr);
    glCompileShader(shader);

    auto shaderMessage = GetShaderInfoLog(shader);
	if (shaderMessage != "")
		spdlog::info(shaderMessage);
}

void Resources::ShaderManager::LoadShader(GLuint program, const ResourcePath& path, Shaders::ShaderType t) {
    GLuint shaderType;
    switch (t) {
        case ShaderType::VERT:
            shaderType = GL_VERTEX_SHADER;
            break;
        case ShaderType::FRAG:
            shaderType = GL_FRAGMENT_SHADER;
            break;
        case ShaderType::GEOM:
            shaderType = GL_GEOMETRY_SHADER;
            break;
        case ShaderType::VERT_FRAG:
            LoadShader(program, path.GetUnparsedPathStr() + EXT_VERT, ShaderType::VERT);
            LoadShader(program, path.GetUnparsedPathStr() + EXT_FRAG, ShaderType::FRAG);
            return;
        case ShaderType::VERT_FRAG_GEOM:
            LoadShader(program, path.GetUnparsedPathStr() + EXT_VERT, ShaderType::VERT);
            LoadShader(program, path.GetUnparsedPathStr() + EXT_FRAG, ShaderType::FRAG);
            LoadShader(program, path.GetUnparsedPathStr() + EXT_GEOM, ShaderType::GEOM);
            return;
        default:
            return;
    }
    GLuint shader = glCreateShader(shaderType);
    spdlog::info("Compiling shader '" + std::fs::path(path.GetParsedPathStr()).filename().generic_string() + "'");
    LoadShaderFromFile(shader, path);
    glAttachShader(program, shader);
    glDeleteShader(shader);
}

void Resources::ShaderManager::LoadStandardShader(Shaders::ShaderID id, const ResourcePath& path, Shaders::ShaderType t) {
    std::string strId = std::string(magic_enum::enum_name(id));
    onResourceLoad.Dispatch(strId);
    GLuint program = glCreateProgram();
    LoadShader(program, ResourcePath("${core_shaders}", path), t);
    glLinkProgram(program);
    auto programMessage = GetProgramInfoLog(program);
	if (programMessage != "")
		spdlog::info(programMessage);
    items_[strId] = program;
}

void Resources::ShaderManager::LoadShader(const std::string& id, const ResourcePath& vert, const ResourcePath& frag, const ResourcePath& geom) {
    onResourceLoad.Dispatch(id);
    GLuint program = glCreateProgram();
    LoadShader(program, vert, ShaderType::VERT);
    LoadShader(program, frag, ShaderType::FRAG);
    if (!geom.IsEmpty())
        LoadShader(program, geom, ShaderType::GEOM);
    glLinkProgram(program);
    auto programMessage = GetProgramInfoLog(program);
	if (programMessage != "")
		spdlog::info(programMessage);
    items_[id] = program;
}

void Resources::ShaderManager::LoadStandardShader(Shaders::ShaderID id, const ResourcePath& vert, const ResourcePath& frag, const ResourcePath& geom) {
    LoadShader(
        (std::string) magic_enum::enum_name(id),
        ResourcePath("${core_shaders}", vert),
        ResourcePath("${core_shaders}", frag),
        geom.IsEmpty() ? geom : ResourcePath("${core_shaders}", geom)
    );
}

void Resources::ShaderManager::LoadStandardShaders() {
    LoadStandardShader(ShaderID::UNLIT, "unlit", ShaderType::VERT_FRAG);
    LoadStandardShader(ShaderID::LIT, "lit", ShaderType::VERT_FRAG);
    LoadStandardShader(ShaderID::FRAMEBUFFER, "framebuffer", ShaderType::VERT_FRAG);
    LoadStandardShader(ShaderID::HIGHLIGHT_NORMALS, "normals", ShaderType::VERT_FRAG_GEOM);
    LoadStandardShader(ShaderID::UI_TEXT, "ui" + EXT_VERT, "text" + EXT_FRAG);
    LoadStandardShader(ShaderID::UI_SHAPE, "ui" + EXT_VERT, "uishape" + EXT_FRAG);
    LoadStandardShader(ShaderID::UI_BLINK, "ui" + EXT_VERT, "blink" + EXT_FRAG);
    LoadStandardShader(ShaderID::LINE, "line", ShaderType::VERT_FRAG_GEOM);
    LoadStandardShader(ShaderID::STROBE_UNLIT, "unlit" + EXT_VERT, "strobe_unlit" + EXT_FRAG);
    LoadStandardShader(ShaderID::SKYBOX, "skybox", ShaderType::VERT_FRAG);
    LoadStandardShader(ShaderID::BILLBOARD, "billboard" + EXT_VERT, "unlit" + EXT_FRAG, "billboard" + EXT_GEOM);
}

void Resources::ShaderManager::Load(const Resources::ShaderImport& import) {
    LoadShader(
        import.id,
        MakeImportPath(import.vertexPath),
        MakeImportPath(import.fragmentPath),
        import.geometryPath.IsEmpty() ? "" : MakeImportPath(import.geometryPath)
    );
}

// tried to template this so the boilerplate wouldn't be needed but instead got the whackiest runtime errors known to man
void Resources::ShaderManager::LoadImports(const Imports<ShaderImport>& imports) {
    SetPath(imports.parentPath);
    for (const auto& import : imports.imports)
        Load(import);
    RestoreDefaultPath();
}

GLuint& Resources::ShaderManager::Get(ShaderID shader) {
    return ResourceTypeManager::Get((std::string) magic_enum::enum_name(shader));
}

std::optional<GLuint> Resources::ShaderManager::LoadResource(const ResourcePath&) {
    throw "not implemented yet";
}

GLuint Shaders::GetShaderProgram(ShaderID shader) {
    return Systems::GetResources().shaderManager.Get(shader);
}

GLuint Shaders::GetShaderProgram(const std::string& shader) {
    return Systems::GetResources().shaderManager.Get(shader);
}

GLuint Shader::GetProgram() const {
    if (program_ == GL_NONE)
        program_ = GetShaderProgram(GetIDString());
    return program_;
}

ShaderID Shader::GetID() const {
    return std::get<ShaderID>(id_);
}

std::string Shader::GetIDString() const {
    if (std::holds_alternative<std::string>(id_))
        return std::get<std::string>(id_);
    else
        return (std::string) magic_enum::enum_name(std::get<ShaderID>(id_));
}

void Shader::Use() const {
    glUseProgram(GetProgram());
}