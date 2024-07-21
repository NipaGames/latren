#include <latren/graphics/texture.h>
#include <latren/io/resourcemanager.h>

#include <stb/stb_image.h>

using namespace Texture;

std::optional<TextureID> Resources::TextureManager::LoadResource(const ResourcePath& path) {
    std::fs::path parsedPath = path.GetParsedPath();
    if (!std::filesystem::exists(parsedPath)) {
        spdlog::warn("Texture does not exist!");
        return std::optional<TextureID>(TEXTURE_NONE);
    }

    TextureID texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, imgChannels;
    uint8_t* data = nullptr;
    data = stbi_load(parsedPath.generic_string().c_str(), &width, &height, &imgChannels, 0);

    if (data == nullptr) {
        spdlog::warn("Can't load texture!");
    }
    // smartass way to convert from n channels to the corresponding opengl format
    GLuint glFormat = (GL_RGB - 3) + imgChannels;
    glTexImage2D(GL_TEXTURE_2D, 0, glFormat, width, height, 0, glFormat, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    return std::optional<TextureID>(texture);
}