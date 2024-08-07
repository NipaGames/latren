#include <latren/physics/debugdrawer.h>
#include <latren/defines/opengl.h>
#include <latren/systems.h>
#include <latren/graphics/renderer.h>
#include <spdlog/spdlog.h>

using namespace Physics;

DebugDrawer::~DebugDrawer() {
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
}

DebugDrawer::DebugDrawer() : shader_(Shaders::ShaderID::LINE) {
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
}

void DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
    vertices_.insert(vertices_.end(), { from.getX(), from.getY(), from.getZ(), to.getX(), to.getY(), to.getZ() });
}

void DebugDrawer::clearLines() {
    vertices_.clear();
}

void DebugDrawer::flushLines() {
    shader_.Use();
    Camera& cam = Systems::GetRenderer().GetCamera();
    shader_.SetUniform("lineColor", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    shader_.SetUniform("view", cam.viewMatrix);
    shader_.SetUniform("projection", cam.projectionMatrix);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(float), vertices_.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glDrawArrays(GL_POINTS, 0, (GLsizei) vertices_.size());
}

void DebugDrawer::reportErrorWarning(const char* msg) {
    spdlog::warn("(DebugDrawer) " + std::string(msg));
}