#pragma once

#include <latren/latren.h>
#include <latren/defines/opengl.h>
#include <iostream>
#include <unordered_map>

#include "shader.h"
#include "texture.h"

#define MATERIAL_MISSING "MATERIAL_MISSING"

class Material {
private:
    std::unordered_map<std::string, int> intUniforms_;
    std::unordered_map<std::string, float> floatUniforms_;

    std::unordered_map<std::string, glm::mat2> mat2Uniforms_;
    std::unordered_map<std::string, glm::mat3> mat3Uniforms_;
    std::unordered_map<std::string, glm::mat4> mat4Uniforms_;

    std::unordered_map<std::string, glm::vec2> vec2Uniforms_;
    std::unordered_map<std::string, glm::vec3> vec3Uniforms_;
    std::unordered_map<std::string, glm::vec4> vec4Uniforms_;

    Shader shader_;
    Texture::TextureID texture_ = TEXTURE_NONE;
public:
    template <typename T>
    struct Uniform {
        std::string name;
        T value;
        Uniform() = default;
        Uniform(const std::string& n, const T& v) : name(n), value(v) { }
    };
    bool cullFaces = true;
    Material() { RestoreDefaultUniforms(); }
    Material(const Shader& shader) : shader_(shader) { RestoreDefaultUniforms(); }
    Material(const Shader& shader, Texture::TextureID textureId) : Material(shader) { texture_ = textureId; }
    template <typename... U>
    Material(const Shader& shader, const Uniform<U>&... uniforms) : shader_(shader) {
        RestoreDefaultUniforms();
        ([&] {
            SetShaderUniform(uniforms.name, uniforms.value);
        } (), ...);
    }
    template <typename... U>
    Material(const Shader& shader, Texture::TextureID textureId, const Uniform<U>&... uniforms) : Material(shader, uniforms...) { texture_ = textureId; }
    LATREN_API void RestoreDefaultUniforms();
    LATREN_API void ClearUniforms();
    LATREN_API void Use(const Shader& shader) const;
    LATREN_API void Use() const;
    template <typename T>
    void SetShader(T s) {
        shader_ = Shader(s);
    }
    LATREN_API void SetTexture(Texture::TextureID t);
    LATREN_API void BindTexture() const;
    const Shader& GetShader() { return shader_; }
    Texture::TextureID GetTexture() { return texture_; }

    template <typename T>
    T& GetShaderUniformReference(const std::string& name) {
        if constexpr(std::is_same_v<T, int>)
            return intUniforms_[name];
        else if constexpr(std::is_same_v<T, float>)
            return floatUniforms_[name];
        else if constexpr(std::is_same_v<T, glm::mat2>)
            return mat2Uniforms_[name];
        else if constexpr(std::is_same_v<T, glm::mat3>)
            return mat3Uniforms_[name];
        else if constexpr(std::is_same_v<T, glm::mat4>)
            return mat4Uniforms_[name];
        else if constexpr(std::is_same_v<T, glm::vec2>)
            return vec2Uniforms_[name];
        else if constexpr(std::is_same_v<T, glm::vec3>)
            return vec3Uniforms_[name];
        else if constexpr(std::is_same_v<T, glm::vec4>)
            return vec4Uniforms_[name];
        else {
            spdlog::error("Invalid uniform type passed! ({})", typeid(T).name());
            throw;
        }
    }

    template <typename T>
    void SetShaderUniform(const std::string& name, const T& value) {
        GetShaderUniformReference<T>(name) = value;
    }
    template <typename T>
    void SetShaderUniform(const Uniform<T>& uniform) {
        SetShaderUniform<T>(uniform.name, uniform.value);
    }
    template <typename T>
    T GetShaderUniform(const std::string& name) {
        return GetShaderUniformReference<T>(name);
    }
};