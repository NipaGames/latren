#pragma once

#include <latren/defines/opengl.h>
#include <vector>
#include <unordered_map>

#include "camera.h"
#include "shader.h"
#include "mesh.h"
#include "shape.h"
#include "viewport.h"
#include <latren/ec/mempool.h>

// forward declarations
class PostProcessing;
namespace UI {
    class Canvas;
};
namespace Lights {
    class ILight;
};
namespace Config {
    struct VideoSettings;
};

class LATREN_API Renderer {
private:
    Viewport* viewport_;
    GLuint fbo_ = GL_NONE;
    GLuint rbo_ = GL_NONE;
    GLuint framebufferTexture_ = GL_NONE;
    GLuint MSAAFbo_ = GL_NONE;
    GLuint MSAATextureColorBuffer_ = GL_NONE;
    Shape framebufferShape_;
    // smart pointers would be ideal here but i'm too lazy and tired to start rewriting
    std::unordered_map<std::string, UI::Canvas*> canvases_;
    Camera camera_ = Camera();
    Shader framebufferShader_;
    Shader normalShader_;
    glm::ivec2 viewportSize_;
    int maxRenderedLights_ = 0;
    std::vector<GLuint> shaders_;
    std::vector<GeneralComponentReference> renderablesOnFrustum_;
    std::unordered_map<std::string, std::shared_ptr<Material>> materials_;
public:
    std::shared_ptr<Mesh> skybox = nullptr;
    Texture::TextureID skyboxTexture = TEXTURE_NONE;
    glm::vec3 skyboxColor = glm::vec3(0.0f);
    bool highlightNormals = false;
    bool showHitboxes = false;
    bool showAabbs = false;

    Renderer() = default;
    Renderer(Viewport*);
    virtual ~Renderer();
    void UpdateLighting();
    void SetViewport(Viewport* viewport) { viewport_ = viewport; }
    Camera& GetCamera() { return camera_; }
    bool Init();
    void Start();
    void Render();
    void UpdateCameraProjection(int, int);
    void CopyShadersFromResources();
    void UpdateFrustum();
    void SortMeshesByDistance();
    void UpdateVideoSettings(const Config::VideoSettings&);
    void ApplyPostProcessing(const PostProcessing&);
    void RestoreViewport();
    // has the ownership and marks it with isOwnedByRenderer
    UI::Canvas& CreateCanvas(std::string);
    UI::Canvas& GetCanvas(const std::string&);
    // doesn't take ownership, but can be given with MoveCanvas
    void AssignCanvas(const std::string&, UI::Canvas*);
    // takes the ownership
    void MoveCanvas(const std::string&, UI::Canvas*);
    void RemoveCanvas(const std::string&);
    void CleanUp();
    std::size_t CountEntitiesOnFrustum() const { return renderablesOnFrustum_.size(); }
    std::shared_ptr<Material> GetMaterial(const std::string&) const;
    std::unordered_map<std::string, std::shared_ptr<Material>>& GetMaterials() { return materials_; }
    const std::vector<GLuint>& GetShaders() const { return shaders_; }
};