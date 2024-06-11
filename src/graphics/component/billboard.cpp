#include <latren/graphics/component/billboard.h>
#include <latren/game.h>

void BillboardRenderer::Delete() {
    if (vao_ != GL_NONE)
        glDeleteVertexArrays(1, &vao_);
    if (vbo_ != GL_NONE)
        glDeleteBuffers(1, &vbo_);
    vao_ = GL_NONE;
    vbo_ = GL_NONE;
}

void BillboardRenderer::UpdateVertexBuffer() {
    glBindVertexArray(vao_);
    const glm::vec3 pos = positions->front();
    float* pointsBuffer = new float[positions->size() * 3];
    pointsCount_ = (GLsizei) positions->size();
    for (int i = 0; i < positions->size(); i++) {
        int ptr = i * 3;
        pointsBuffer[ptr] = positions->at(i).x;
        pointsBuffer[ptr + 1] = positions->at(i).y;
        pointsBuffer[ptr + 2] = positions->at(i).z;
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * pointsCount_, pointsBuffer, GL_STATIC_DRAW);
    delete[] pointsBuffer;
    glBindVertexArray(0);
}

void BillboardRenderer::Start() {
    Renderable::Start();

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    UpdateVertexBuffer();
}

void BillboardRenderer::CalculateMatrices() {
    modelMatrix_ = glm::translate(glm::mat4(1.0f), parent.GetTransform().position.Get());
    modelMatrix_ *= glm::mat4_cast(parent.GetTransform().rotation.Get());
    modelMatrix_ = glm::scale(modelMatrix_, parent.GetTransform().size.Get());
}

void BillboardRenderer::UpdateUniforms(const Shader& shader, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const glm::vec3& viewPos) const {
    Renderable::UpdateUniforms(shader, projectionMatrix, viewMatrix, viewPos);
    shader.SetUniform("model", modelMatrix_);
}

void BillboardRenderer::Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const glm::vec3& viewPos, const Shader* shader, bool aabbDebug) const {
    if (material.Get() != nullptr) {
        UpdateUniforms(GetMaterialShader(material), projectionMatrix, viewMatrix, viewPos);
        material.Get()->Use();
    }
    else {
        UpdateUniforms(BillboardRenderer::SHADER_, projectionMatrix, viewMatrix, viewPos);
    }
    glBindVertexArray(vao_);
    glDrawArrays(GL_POINTS, 0, pointsCount_);
}