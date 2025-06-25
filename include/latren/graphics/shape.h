#pragma once

#include <latren/latren.h>
#include <latren/defines/opengl.h>

class  Shape {
private:
    float* vertexData = nullptr;
public:
    int numVertices = 6;
    int numVertexAttributes = 2;
    int stride = 2;
    GLuint vao = GL_NONE;
    GLuint vbo = GL_NONE;
    
    virtual ~Shape();
    Shape() = default;
    Shape(const Shape&);
    Shape(Shape&&);
    Shape& operator=(const Shape&);
    Shape& operator=(Shape&&);
    void DeleteBuffers();
    void GenerateVAO();
    void Bind() const;
    void SetVertexData(float*, bool);
    void SetVertexData(const float*);
};

namespace Shapes {
    enum class DefaultShape {
        RECTANGLE_VEC4,
        RECTANGLE_VEC2_VEC2
    };
     void CreateDefaultShapes();
    const Shape& GetDefaultShape(DefaultShape);
};