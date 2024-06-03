#include <latren/graphics/mesh.h>
#include <latren/graphics/texture.h>

#include <iostream>
#include <spdlog/spdlog.h>

Mesh::Mesh(const std::vector<float>& v, const std::vector<unsigned int>& i, const std::vector<float>& t, const std::vector<float>& n) : vertices(v), indices(i), texCoords(t), normals(n) { }
Mesh::Mesh(const std::vector<float>& v, const std::vector<unsigned int>& i, const std::vector<float>& t) : vertices(v), texCoords(t), indices(i), normals(v) { }
Mesh::Mesh(const std::vector<float>& v, const std::vector<unsigned int>& i) : vertices(v), indices(i), normals(v) {
    for (int i = 0; i < vertices.size() / 3; i++) {
        texCoords.push_back(0.0f);
        texCoords.push_back(0.0f);
    }
}
Mesh::Mesh(const std::string& meshId, const std::vector<float>& v, const std::vector<unsigned int>& i, const std::vector<float>& t) : Mesh(v, i, t) { id = meshId; }
Mesh::Mesh(const std::string& meshId, const std::vector<float>& v, const std::vector<unsigned int>& i) : Mesh(v, i) { id = meshId; }
Mesh::Mesh(const Mesh& m) : id(m.id), vertices(m.vertices), indices(m.indices), texCoords(m.texCoords), normals(m.normals) { }
Mesh::Mesh(Mesh&& m) :
    id(m.id),
    vertices(m.vertices),
    indices(m.indices),
    texCoords(m.texCoords),
    normals(m.normals),
    vao(m.vao),
    vbo(m.vbo),
    ebo(m.ebo),
    normalBuffer(m.normalBuffer),
    texCoordBuffer(m.texCoordBuffer)
{
    m.vao = GL_NONE;
    m.vbo = GL_NONE;
    m.ebo = GL_NONE;
    m.normalBuffer = GL_NONE;
    m.texCoordBuffer = GL_NONE;
}

void Mesh::GenerateVAO() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &texCoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(texCoords[0]), texCoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(normals[0]), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
}

void Mesh::Bind() const {
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
}

void Mesh::Render() const {
    if (!cullFaces)
        glDisable(GL_CULL_FACE);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
}

std::shared_ptr<Mesh> Meshes::CreateMeshInstance(const Mesh& m) {
    auto mesh = std::make_shared<Mesh>(m);
    mesh->GenerateVAO();
    return mesh;
}

Mesh::~Mesh() {
    if (vao != GL_NONE)
        glDeleteVertexArrays(1, &vao);
    if (vbo != GL_NONE)
        glDeleteBuffers(1, &vbo);
    if (ebo != GL_NONE)
        glDeleteBuffers(1, &ebo);
    if (texCoordBuffer != GL_NONE)
        glDeleteBuffers(1, &texCoordBuffer);
    if (normalBuffer != GL_NONE)
        glDeleteBuffers(1, &normalBuffer);
    vao = GL_NONE;
    vbo = GL_NONE;
    ebo = GL_NONE;
    texCoordBuffer = GL_NONE;
    normalBuffer = GL_NONE;
}