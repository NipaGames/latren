#include <latren/graphics/renderer.h>
#include <latren/graphics/postprocessing.h>
#include <latren/graphics/component/light.h>
#include <latren/graphics/component/renderable.h>
#include <latren/systems.h>
#include <latren/gamewindow.h>
#include <latren/physics/physics.h>
#include <latren/ui/canvas.h>
#include <latren/io/resourcemanager.h>
#include <latren/io/configs.h>

#include <spdlog/spdlog.h>

Renderer::Renderer(Viewport* window) {
    SetViewport(window);
}

Renderer::~Renderer() {
    for (GLuint s : shaders_)
        glDeleteProgram(s);

    glDeleteFramebuffers(1, &fbo_);
    glDeleteRenderbuffers(1, &rbo_);
    glDeleteTextures(1, &framebufferTexture_);
    glDeleteFramebuffers(1, &MSAAFbo_);
    glDeleteTextures(1, &MSAATextureColorBuffer_);

    shaders_.clear();

    for (const auto& [k, v] : canvases_) {
        if (v->isOwnedByRenderer)
            delete v;
    }
    canvases_.clear();
}

bool Renderer::Init() {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

    glfwWindowHint(GLFW_SAMPLES, 4);
    glEnable(GL_MULTISAMPLE);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // debug line rendering

    float aspectRatio = 16.0f / 9.0f;

    camera_.aspectRatio = aspectRatio;
    camera_.projectionMatrix = glm::perspective(glm::radians(camera_.fov), aspectRatio, camera_.clippingNear, camera_.clippingFar);
    camera_.pos = glm::vec3(0.0f);

    viewportSize_ = viewport_->GetSize();

    glGenFramebuffers(1, &MSAAFbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, MSAAFbo_);

    glGenTextures(1, &MSAATextureColorBuffer_);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, MSAATextureColorBuffer_);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, viewportSize_.x, viewportSize_.y, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, MSAATextureColorBuffer_, 0);

    glGenRenderbuffers(1, &rbo_);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, viewportSize_.x, viewportSize_.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_);

    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

    glGenTextures(1, &framebufferTexture_);
    glBindTexture(GL_TEXTURE_2D, framebufferTexture_);
    Systems::GetResources().GetTextureManager()->Set("FRAMEBUFFER", framebufferTexture_);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewportSize_.x, viewportSize_.y, 0, GL_RGB, GL_UNSIGNED_BYTE, GL_NONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture_, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    framebufferShader_ = Shader(Shaders::ShaderID::FRAMEBUFFER);

    Shapes::CreateDefaultShapes();

    framebufferShape_ = Shapes::GetDefaultShape(Shapes::DefaultShape::RECTANGLE_VEC2_VEC2);
    const float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    framebufferShape_.SetVertexData(quadVertices);

    std::shared_ptr<Material> missingMaterial = std::make_shared<Material>(Shaders::ShaderID::STROBE_UNLIT);
    missingMaterial->SetShaderUniform("colors[0]", glm::vec3(0.0f));
    missingMaterial->SetShaderUniform("colors[1]", glm::vec3(1.0f, 0.0f, 1.0f));
    missingMaterial->SetShaderUniform("strobeInterval", .25f);
    missingMaterial->SetShaderUniform("colorCount", 2);
    materials_[MATERIAL_MISSING] = missingMaterial;

    return true;
}

void Renderer::UpdateLighting() {
    for (GLuint shader : shaders_) {
        glUseProgram(shader);
        Lights::ResetIndices();
        Systems::GetEntityManager().GetComponentMemory().ForEachDerivedComponent<Lights::ILight>([&](Lights::ILight& l, IComponentMemoryPool&) {
            while (Lights::IsReserved(Lights::LIGHTS_INDEX)) {
                Lights::LIGHTS_INDEX++;
            }
            l.UseAsNext();
            l.ApplyLight(shader);
        });
        if (Lights::LIGHTS_INDEX < maxRenderedLights_) {
            for (int i = Lights::LIGHTS_INDEX; i < maxRenderedLights_; i++) {
                if (!Lights::IsReserved(i))
                    glUniform1i(glGetUniformLocation(shader, std::string("lights[" + std::to_string(i) + "].enabled").c_str()), GL_FALSE);
            }
        }
    }
    maxRenderedLights_ = std::max(Lights::LIGHTS_INDEX, maxRenderedLights_);
}

void Renderer::SetViewport(Viewport* viewport) {
    viewport_ = viewport;
}

Camera& Renderer::GetCamera() {
    return camera_;
}

void Renderer::CopyShadersFromResources() {
    const auto& shaderMap = Systems::GetResources().GetShaderManager()->GetAll();
    shaders_.clear();
    std::transform(shaderMap.begin(), shaderMap.end(), std::back_inserter(shaders_), [](const auto& s) { return s.second; });
}

void Renderer::Start() {
    CopyShadersFromResources();
    UpdateLighting();
    UpdateFrustum();
}

void Renderer::SortMeshesByDistance() {
    std::sort(renderablesOnFrustum_.begin(), renderablesOnFrustum_.end(), [&](GeneralComponentReference& r1, GeneralComponentReference& r2) {
        glm::vec3 pos1 = r1.CastComponent<IRenderable>().GetPosition();
        glm::vec3 pos2 = r2.CastComponent<IRenderable>().GetPosition();
        return glm::distance(camera_.pos, pos1) > glm::distance(camera_.pos, pos2);
    });
}

void Renderer::UpdateFrustum() {
    renderablesOnFrustum_.clear();
    Systems::GetEntityManager().GetComponentMemory().ForEachDerivedComponent<IRenderable>([&](IRenderable& r, IComponentMemoryPool& pool) {
        if (r.IsAlwaysOnFrustum() || r.IsOnFrustum(camera_.frustum))
            renderablesOnFrustum_.push_back({ &pool, static_cast<IComponent&>(r) });
    });
}

void Renderer::Render() {
    // first pass (draw into framebuffer)
    glBindFramebuffer(GL_FRAMEBUFFER, MSAAFbo_);
    
    glEnable(GL_DEPTH_TEST);

    glClearColor(skyboxColor.r, skyboxColor.g, skyboxColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(0);
    Systems::GetEntityManager().GetComponentMemory().ForEachDerivedComponent<IRenderable>([&](IRenderable& r, IComponentMemoryPool&) {
        if (!r.IsStatic())
            r.CalculateMatrices();
    });
    // todo: cache these
    for (auto& pass : renderPasses_) {
        pass.clear();
    }
    auto it = renderablesOnFrustum_.begin();
    while (it != renderablesOnFrustum_.end()) {
        GeneralComponentReference& ref = *it;
        if (ref.IsNull()) {
            it = renderablesOnFrustum_.erase(it);
            continue;
        }
        renderPasses_[ref.CastComponent<IRenderable>().GetRenderPass()].push_back(ref);
        ++it;
    }

    DoRenderPass(RenderPass::NORMAL);

    // draw skybox
    if (skybox != nullptr) {
        glDepthFunc(GL_LEQUAL);
        glCullFace(GL_FRONT);
        glEnable(GL_DEPTH_CLAMP);

        const Shader* shader = &skybox->material->GetShader();
        shader->Use();
        glm::mat4 skyboxView = glm::mat4(glm::mat3(camera_.viewMatrix));
        shader->SetUniform("view", skyboxView);
        shader->SetUniform("projection", camera_.projectionMatrix);
        shader->SetUniform("clippingFar", camera_.clippingFar);
        
        glBindVertexArray(skybox->vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skybox->ebo);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

        skybox->Render();
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
        glCullFace(GL_BACK);
        glDisable(GL_DEPTH_CLAMP);
    }
    
    DoRenderPass(RenderPass::LATE);
    
    if (showAabbs)
        DebugDrawAABBs();
    if (highlightNormals)
        DebugDrawNormals();
    if (showHitboxes)
        DebugDrawHitboxes();

    // second pass (draw framebuffer onto screen)
    glBindFramebuffer(GL_READ_FRAMEBUFFER, MSAAFbo_);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_);
    glBlitFramebuffer(0, 0, viewportSize_.x, viewportSize_.y, 0, 0, viewportSize_.x, viewportSize_.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    framebufferShader_.Use();
    framebufferShape_.Bind();
    glBindTexture(GL_TEXTURE_2D, framebufferTexture_);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    DoRenderPass(RenderPass::AFTER_POST_PROCESSING);
    glDisable(GL_DEPTH_TEST);
    for (auto& c : canvases_) {
        c.second->Update();
        c.second->Draw();
    }

    glEnable(GL_DEPTH_TEST);
}

void Renderer::RenderItem(IRenderable& renderable, int renderMode) {
    renderable.IRender(camera_.projectionMatrix, camera_.viewMatrix, camera_.pos, nullptr, renderMode);
}

void Renderer::DoRenderPass(RenderPass::Enum pass) {
    for (GeneralComponentReference& ref : renderPasses_[pass]) {
        RenderItem(ref.CastComponent<IRenderable>());
    }
}

void Renderer::RestoreViewport() {
    glViewport(0, 0, viewportSize_.x, viewportSize_.y);
}

void Renderer::UpdateCameraProjection(int width, int height) {
    viewportSize_ = glm::ivec2(width, height);
    glViewport(0, 0, width, height);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, MSAATextureColorBuffer_);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, width, height, GL_TRUE);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
    glBindTexture(GL_TEXTURE_2D, framebufferTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, GL_NONE);

    camera_.aspectRatio = (float) width / (float) height;
    camera_.projectionMatrix = glm::perspective(glm::radians(camera_.fov), camera_.aspectRatio, camera_.clippingNear, camera_.clippingFar);
}

void Renderer::ApplyPostProcessing(const PostProcessing& postProcessing) {
    framebufferShader_.Use();
    postProcessing.ApplyUniforms(framebufferShader_);
    glUseProgram(0);
}

void Renderer::UpdateVideoSettings(const Config::VideoSettings& settings) {
    framebufferShader_.Use();
    framebufferShader_.SetUniform("cfg.gamma", settings.gamma);
    framebufferShader_.SetUniform("cfg.contrast", settings.contrast);
    framebufferShader_.SetUniform("cfg.brightness", settings.brightness);
    framebufferShader_.SetUniform("cfg.saturation", settings.saturation);
    glUseProgram(0);

    camera_.fov = settings.fov;

    glm::ivec2 wndSize = viewport_->GetSize();
    if (wndSize.x > 0 && wndSize.y > 0)
        UpdateCameraProjection(wndSize.x, wndSize.y);
}

UI::Canvas& Renderer::CreateCanvas(std::string id) {
    UI::Canvas* c = new UI::Canvas();
    c->isOwnedByRenderer = true;
    auto [ it, inserted ] = canvases_.insert({ id, c });
    it->second->GenerateBackgroundShape();
    return *it->second;
}

UI::Canvas& Renderer::GetCanvas(const std::string& id) {
    return *canvases_.at(id);
}

void Renderer::AssignCanvas(const std::string& id, UI::Canvas* c) {
    canvases_.insert({ id, c });
    glm::ivec2 wndSize = viewport_->GetSize();
}

void Renderer::RemoveCanvas(const std::string& id) {
    auto it = canvases_.find(id);
    if (it == canvases_.end())
        return;
    canvases_.erase(it);
}

void Renderer::MoveCanvas(const std::string& id, UI::Canvas* c) {
    AssignCanvas(id, c);
    c->isOwnedByRenderer = true;
}

void Renderer::CleanUp() {
    for (const auto& [k, v] : canvases_) {
        if (v->isOwnedByRenderer)
            delete v;
    }
    canvases_.clear();
    renderablesOnFrustum_.clear();
    UpdateFrustum();
}

std::size_t Renderer::CountEntitiesOnFrustum() const {
    return renderablesOnFrustum_.size();
}

void Renderer::ForEachRenderableOnFrustum(const std::function<void(IRenderable&)>& fn) {
    for (GeneralComponentReference& renderable : renderablesOnFrustum_) {
        fn(renderable.CastComponent<IRenderable>());
    }
}

std::shared_ptr<Material> Renderer::GetMaterial(const std::string& mat) const {
    if (materials_.find(mat) == materials_.end())
        return materials_.at(MATERIAL_MISSING);
    return materials_.at(mat);
}

std::unordered_map<std::string, std::shared_ptr<Material>>& Renderer::GetMaterials() {
    return materials_;
}

const std::vector<GLuint>& Renderer::GetShaders() const {
    return shaders_;
}

void Renderer::DebugDrawNormals() {
    Systems::GetEntityManager().GetComponentMemory().ForEachDerivedComponent<IRenderable>([&](IRenderable& r, IComponentMemoryPool&) {
        RenderItem(r, RENDER_MODE_DEBUG_NORMALS);
    });
}

void Renderer::DebugDrawHitboxes() {
    if (Systems::GetPhysics().GetDynamicsWorld() != nullptr)
        Systems::GetPhysics().GetDynamicsWorld()->debugDrawWorld();
}

void Renderer::DebugDrawAABBs() {
    Systems::GetEntityManager().GetComponentMemory().ForEachDerivedComponent<IRenderable>([&](IRenderable& r, IComponentMemoryPool&) {
        RenderItem(r, RENDER_MODE_DEBUG_AABBS);
    });
}