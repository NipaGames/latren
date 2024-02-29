#pragma once

#include <latren/defines/opengl.h>
#include <vector>
#include <unordered_map>

#include "camera.h"
#include "shader.h"
#include "mesh.h"
#include "shape.h"
#include "component/renderable.h"

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

class Renderer {
private:
    GLFWwindow* window_;
    GLuint fbo_;
    GLuint rbo_;
    GLuint framebufferTexture_;
    GLuint MSAAFbo_;
    GLuint MSAATextureColorBuffer_;
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
    Texture::TextureID skyboxTexture = NULL;
    glm::vec3 skyboxColor = glm::vec3(0.0f);
    bool highlightNormals = false;
    bool showHitboxes = false;
    bool showAabbs = false;
    Renderer() = default;
    LATREN_API virtual ~Renderer();
    LATREN_API void UpdateLighting();
    void SetWindow(GLFWwindow* window) { window_ = window; }
    LATREN_API Renderer(GLFWwindow*);
    Camera& GetCamera() { return camera_; }
    LATREN_API bool Init();
    LATREN_API void Start();
    LATREN_API void Render();
    LATREN_API void UpdateCameraProjection(int, int);
    LATREN_API void CopyShadersFromResources();
    LATREN_API void UpdateFrustum();
    LATREN_API void SortMeshesByDistance();
    LATREN_API void UpdateVideoSettings(const Config::VideoSettings&);
    LATREN_API void ApplyPostProcessing(const PostProcessing&);
    LATREN_API void RestoreViewport();
    // has the ownership and marks it with isOwnedByRenderer
    LATREN_API UI::Canvas& CreateCanvas(std::string);
    LATREN_API UI::Canvas& GetCanvas(const std::string&);
    // doesn't take ownership, but can be given with MoveCanvas
    LATREN_API void AssignCanvas(const std::string&, UI::Canvas*);
    // takes the ownership
    LATREN_API void MoveCanvas(const std::string&, UI::Canvas*);
    LATREN_API void RemoveCanvas(const std::string&);
    LATREN_API void CleanUp();
    size_t CountEntitiesOnFrustum() { return renderablesOnFrustum_.size(); }
    LATREN_API std::shared_ptr<Material> GetMaterial(const std::string&);
    std::unordered_map<std::string, std::shared_ptr<Material>>& GetMaterials() { return materials_; }
    const std::vector<GLuint>& GetShaders() { return shaders_; }
};