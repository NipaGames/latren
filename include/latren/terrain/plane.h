#pragma once

#include <latren/defines/opengl.h>
#include <latren/graphics/mesh.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

class Plane : public Mesh {
private:
    glm::ivec2 tiling_;
    std::vector<std::vector<float>> heightGrid_;
public:
    float variation = 0.0f;
    float heightVariation = 0.0f;
    Plane() : tiling_(1, 1) { }
    Plane(const glm::ivec2& t) : tiling_(t) { }
    LATREN_API  void GenerateVertices();
    LATREN_API std::shared_ptr<btHeightfieldTerrainShape> CreateBtCollider() const;
};