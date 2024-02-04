#pragma once

#include <latren/latren.h>
#include <latren/defines/opengl.h>

class Shape {
private:
    float* vertexData = nullptr;
public:
    int numVertices = 6;
    int numVertexAttributes = 2;
    int stride = 2;
    GLuint vao = NULL;
    GLuint vbo = NULL;
    LATREN_API ~Shape();
    Shape() = default;
    LATREN_API Shape(const Shape&);
    LATREN_API Shape(Shape&&);
    LATREN_API Shape& operator=(const Shape&);
    LATREN_API Shape& operator=(Shape&&);
    LATREN_API void GenerateVAO();
    LATREN_API void Bind();
    LATREN_API void SetVertexData(float*, bool);
    LATREN_API void SetVertexData(const float*);
};

namespace Shapes {
    inline Shape RECTANGLE_VEC4;
    inline Shape RECTANGLE_VEC2_VEC2;
    LATREN_API void CreateDefaultShapes();
};