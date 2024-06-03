#pragma once

#include <latren/defines/opengl.h>
#include <memory>
#include <string>
#include <vector>

#include "material.h"
#include "shader.h"
#include "camera.h"

class LATREN_API Mesh {
public:
    GLuint vao = GL_NONE;
    GLuint vbo = GL_NONE;
    GLuint ebo = GL_NONE;
    GLuint normalBuffer = GL_NONE;
    GLuint texCoordBuffer = GL_NONE;
    glm::mat4 transformMatrix = glm::mat4(1.0f);
    ViewFrustum::AABB aabb;
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texCoords;
    std::vector<unsigned int> indices;
    std::shared_ptr<Material> material;
    std::string id;
    bool cullFaces = true;

    virtual ~Mesh();
    Mesh() = default;
    Mesh(const std::vector<float>&, const std::vector<unsigned int>&, const std::vector<float>&, const std::vector<float>&);
    Mesh(const std::vector<float>&, const std::vector<unsigned int>&, const std::vector<float>&);
    Mesh(const std::vector<float>&, const std::vector<unsigned int>&);
    Mesh(const std::string&, const std::vector<float>&, const std::vector<unsigned int>&, const std::vector<float>&);
    Mesh(const std::string&, const std::vector<float>&, const std::vector<unsigned int>&);
    Mesh(const Mesh&);
    Mesh(Mesh&&);
    virtual void GenerateVAO();
    virtual void Render() const;
    virtual void Bind() const;
};

namespace Meshes {
    LATREN_API std::shared_ptr<Mesh> CreateMeshInstance(const Mesh&);
    static Mesh CUBE(
        "cube",
        {
            -0.5f, -0.5f,  0.5f,    0.5f, -0.5f,  0.5f,    0.5f,  0.5f,  0.5f,   -0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,    0.5f,  0.5f, -0.5f,    0.5f, -0.5f, -0.5f,   -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f,  0.5f,    0.5f, -0.5f, -0.5f,    0.5f,  0.5f, -0.5f,    0.5f,  0.5f,  0.5f,
            -0.5f, -0.5f, -0.5f,   -0.5f, -0.5f,  0.5f,   -0.5f,  0.5f,  0.5f,   -0.5f,  0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,   -0.5f, -0.5f, -0.5f,   -0.5f,  0.5f, -0.5f,    0.5f,  0.5f, -0.5f
        },
        {
            0,  1,  2,  2,  3,  0,
            19, 16, 17, 17, 18, 19,
            3,  2,  5,  5,  4,  3,
            7,  6,  1,  1,  0,  7,
            8,  9,  10, 10, 11, 8,
            12, 13, 14, 14, 15, 12
        },
        {
            0.0f, 1.0f,  1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,
            0.0f, 1.0f,  1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,
            0.0f, 1.0f,  1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,
            0.0f, 1.0f,  1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,
            0.0f, 1.0f,  1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f
        }
    );

    static Mesh CUBE_WITHOUT_TEXCOORDS(
        "cube_notex",
        {
            -.5f,    .5f,    .5f,
            -.5f,   -.5f,    .5f,
             .5f,    .5f,    .5f,
             .5f,   -.5f,    .5f,
            -.5f,    .5f,   -.5f,
            -.5f,   -.5f,   -.5f,
             .5f,    .5f,   -.5f,
             .5f,   -.5f,   -.5f
        },
        {
            3, 2, 0, 1, 3, 0,
            7, 6, 2, 3, 7, 2,
            5, 4, 6, 7, 5, 6,
            1, 0, 4, 5, 1, 4,
            6, 4, 0, 2, 6, 0,
            1, 5, 7, 1, 7, 3,
        }
    );
};