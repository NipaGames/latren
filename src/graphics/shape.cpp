#include <latren/graphics/shape.h>

#include <cstring>

void Shape::GenerateVAO() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numVertices * numVertexAttributes, NULL, GL_DYNAMIC_DRAW);
    for (int i = 0; i < numVertexAttributes / stride; i++) {
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, stride, GL_FLOAT, GL_FALSE, numVertexAttributes * sizeof(float), reinterpret_cast<void*>(i * stride * sizeof(float)));
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Shape::Bind() {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
}

Shape::Shape(const Shape& s) : 
    numVertexAttributes(s.numVertexAttributes),
    numVertices(s.numVertices),
    stride(s.stride)
{
    if (s.vao != NULL)
        GenerateVAO();
    if (s.vertexData != nullptr)
        SetVertexData(s.vertexData);
}

Shape::Shape(Shape&& s) :
    numVertexAttributes(s.numVertexAttributes),
    numVertices(s.numVertices),
    stride(s.stride),
    vao(s.vao),
    vbo(s.vbo),
    vertexData(s.vertexData)
{
    s.vertexData = nullptr;
    s.vao = NULL;
    s.vbo = NULL;
}

Shape::~Shape() {
    if (vao != NULL)
        glDeleteVertexArrays(1, &vao);
    if (vbo != NULL)
        glDeleteBuffers(1, &vbo);
    if (vertexData != nullptr)
        delete[] vertexData;
    vertexData = nullptr;
}

Shape& Shape::operator=(const Shape& s) {
    Shape::~Shape();
    
    numVertexAttributes = s.numVertexAttributes;
    numVertices = s.numVertices;
    stride = s.stride;

    if (s.vao != NULL)
        GenerateVAO();
    if (s.vertexData != nullptr)
        SetVertexData(s.vertexData);
    
    return *this;
}

Shape& Shape::operator=(Shape&& s) {
    Shape::~Shape();
    
    numVertexAttributes = s.numVertexAttributes;
    numVertices = s.numVertices;
    stride = s.stride;
    vao = s.vao;
    vbo = s.vbo;
    vertexData = s.vertexData;

    s.vertexData = nullptr;
    s.vao = NULL;
    s.vbo = NULL;
    
    return *this;
}

void Shape::SetVertexData(float* f, bool copy) {
    size_t s = sizeof(float) * numVertices * numVertexAttributes;
    if (vertexData != nullptr)
        delete[] vertexData;
    if (copy) {
        vertexData = new float[s];
        memcpy_s(vertexData, s, f, s);
    }
    else {
        vertexData = f;
    }
    Bind();
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numVertices * numVertexAttributes, f, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Shape::SetVertexData(const float* f) {
    SetVertexData(const_cast<float*>(f), true);
}

void Shapes::CreateDefaultShapes() {
    RECTANGLE_VEC4.numVertices = 6;
    RECTANGLE_VEC4.numVertexAttributes = 4;
    RECTANGLE_VEC4.stride = 4;
    RECTANGLE_VEC4.GenerateVAO();

    RECTANGLE_VEC2_VEC2.numVertices = 6;
    RECTANGLE_VEC2_VEC2.numVertexAttributes = 4;
    RECTANGLE_VEC2_VEC2.stride = 2;
    RECTANGLE_VEC2_VEC2.GenerateVAO();
}