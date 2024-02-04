#include <latren/graphics/renderer.h>
#include <latren/graphics/postprocessing.h>
#include <latren/graphics/component/light.h>
#include <latren/graphics/component/renderable.h>
#include <latren/game.h>
#include <latren/gamewindow.h>
#include <latren/physics/physics.h>
#include <latren/ui/canvas.h>

#include <spdlog/spdlog.h>

Renderer::Renderer(GLFWwindow* window) {
    SetWindow(window);
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
    camera_.pos = glm::vec3(0.0f, 0.0f, -10.0f);
    
    glGenFramebuffers(1, &MSAAFbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, MSAAFbo_);

    glGenTextures(1, &MSAATextureColorBuffer_);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, MSAATextureColorBuffer_);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, BASE_WIDTH, BASE_HEIGHT, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, MSAATextureColorBuffer_, 0);

    glGenRenderbuffers(1, &rbo_);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, BASE_WIDTH, BASE_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_);

    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

    glGenTextures(1, &framebufferTexture_);
    glBindTexture(GL_TEXTURE_2D, framebufferTexture_);
    Game::GetGameInstanceBase()->GetResources().textureManager.Set("FRAMEBUFFER", framebufferTexture_);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, BASE_WIDTH, BASE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture_, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    framebufferShader_ = Shader(Shaders::ShaderID::FRAMEBUFFER);
    normalShader_ = Shader(Shaders::ShaderID::HIGHLIGHT_NORMALS);

    Shapes::CreateDefaultShapes();

    framebufferShape_ = Shapes::RECTANGLE_VEC2_VEC2;
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

    glfwGetWindowSize(window_, &viewportSize_.x, &viewportSize_.y);

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
        for (auto l : lights_) {
            while (Lights::IsReserved(Lights::LIGHTS_INDEX)) {
                Lights::LIGHTS_INDEX++;
            }
            l->UseAsNext();
            l->ApplyLight(shader);
        }
        if (Lights::LIGHTS_INDEX < maxRenderedLights_) {
            for (int i = Lights::LIGHTS_INDEX; i < maxRenderedLights_; i++) {
                if (!Lights::IsReserved(i))
                    glUniform1i(glGetUniformLocation(shader, std::string("lights[" + std::to_string(i) + "].enabled").c_str()), GL_FALSE);
            }
        }
    }
    maxRenderedLights_ = std::max(Lights::LIGHTS_INDEX, maxRenderedLights_);
}

void Renderer::CopyShadersFromResources() {
    const auto& shaderMap = Game::GetGameInstanceBase()->GetResources().shaderManager.GetAll();
    shaders_.clear();
    std::transform(shaderMap.begin(), shaderMap.end(), std::back_inserter(shaders_), [](const auto& s) { return s.second; });
}

void Renderer::Start() {
    CopyShadersFromResources();
    UpdateLighting();
    UpdateFrustum();
}

void Renderer::SortMeshesByDistance() {
    std::sort(renderablesOnFrustum_.begin(), renderablesOnFrustum_.end(), [&] (const auto& mesh0, const auto& mesh1) {
        return glm::length(camera_.pos - mesh0->parent->transform->position) > glm::length(camera_.pos - mesh1->parent->transform->position);
    });
}

void Renderer::UpdateFrustum() {
    renderablesOnFrustum_.clear();
    std::copy_if(renderables_.begin(), renderables_.end(), std::back_inserter(renderablesOnFrustum_), [&] (const Renderable* renderable) {
        return renderable->alwaysOnFrustum || renderable->IsOnFrustum(camera_.frustum);
    });
}

void Renderer::Render() {
    // first pass (draw into framebuffer)
    glBindFramebuffer(GL_FRAMEBUFFER, MSAAFbo_);
    
    glEnable(GL_DEPTH_TEST);

    glClearColor(skyboxColor.r, skyboxColor.g, skyboxColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 viewMatrix = glm::lookAt(camera_.pos, camera_.pos + camera_.front, camera_.up);

    glUseProgram(0);
    for (auto renderable : renderables_) {
        if (!renderable->isStatic)
            renderable->CalculateMatrices();
    }
    std::vector<const Renderable*> lateRenderables;
    std::vector<const Renderable*> renderablesAfterPostProcessing;
    for (const Renderable* renderable : renderablesOnFrustum_) {
        if (renderable->renderLate) {
            lateRenderables.push_back(renderable);
            continue;
        }
        else if (renderable->renderAfterPostProcessing) {
            renderablesAfterPostProcessing.push_back(renderable);
            continue;
        }
        renderable->IRender(camera_.projectionMatrix, viewMatrix, nullptr, showAabbs);
    }

    // draw skybox
    if (skybox != nullptr) {
        glDepthFunc(GL_LEQUAL);
        glCullFace(GL_FRONT);
        glEnable(GL_DEPTH_CLAMP);

        const Shader* shader = &skybox->material->GetShader();
        shader->Use();
        glm::mat4 skyboxView = glm::mat4(glm::mat3(viewMatrix));
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

    for (const Renderable* renderable : lateRenderables) {
         renderable->IRender(camera_.projectionMatrix, viewMatrix, nullptr, showAabbs);
    }

    if (highlightNormals) {
        for (const Renderable*  renderable : renderables_) {
            renderable->IRender(camera_.projectionMatrix, viewMatrix, &normalShader_);
        }
    }
    if (showHitboxes) {
        if (Physics::GLOBAL_DYNAMICS_WORLD_ != nullptr)
            Physics::GLOBAL_DYNAMICS_WORLD_->debugDrawWorld();
    }

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
    for (const Renderable* renderable : renderablesAfterPostProcessing) {
        renderable->IRender(camera_.projectionMatrix, viewMatrix, nullptr, showAabbs);
    }
    
    glDisable(GL_DEPTH_TEST);
    for (auto& c : canvases_) {
        c.second->Draw();
    }

    glEnable(GL_DEPTH_TEST);
    glfwSwapBuffers(window_);
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    camera_.aspectRatio = (float) width / (float) height;
    camera_.projectionMatrix = glm::perspective(glm::radians(camera_.fov), camera_.aspectRatio, camera_.clippingNear, camera_.clippingFar);
    
    for (auto& c : canvases_) {
        c.second->UpdateWindowSize(width, height);
    }
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

    int w, h;
    glfwGetWindowSize(window_, &w, &h);
    UpdateCameraProjection(w, h);
}

void Renderer::AddLight(Lights::Light* light) {
    lights_.push_back(light);
    light->isAssignedToRenderer_ = true;
}

void Renderer::RemoveLight(Lights::Light* light) {
    light->isAssignedToRenderer_ = false;
    if (lights_.size() == 0) return;
    lights_.erase(std::remove(lights_.begin(), lights_.end(), light), lights_.end());
}

void Renderer::AddRenderable(Renderable* renderable) {
    renderables_.push_back(renderable);
    renderable->isAssignedToRenderer_ = true;
}

void Renderer::RemoveRenderable(Renderable* renderable) {
    if (renderables_.size() == 0) return;
    renderables_.erase(std::remove(renderables_.begin(), renderables_.end(), renderable), renderables_.end());
    UpdateFrustum();
}

void Renderer::CleanUpEntities() {
    for (const auto& [k, v] : canvases_) {
        if (v->isOwnedByRenderer)
            delete v;
    }
    canvases_.clear();
    renderables_.clear();
    renderablesOnFrustum_.clear();
    lights_.clear();
    UpdateFrustum();
}

std::shared_ptr<Material> Renderer::GetMaterial(const std::string& mat) {
    if (materials_.count(mat) == 0)
        return materials_.at(MATERIAL_MISSING);
    return materials_.at(mat);
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
    glm::ivec2 wndSize;
    glfwGetWindowSize(window_, &wndSize.x, &wndSize.y);
    c->UpdateWindowSize(wndSize.x, wndSize.y);
}

void Renderer::MoveCanvas(const std::string& id, UI::Canvas* c) {
    AssignCanvas(id, c);
    c->isOwnedByRenderer = true;
}