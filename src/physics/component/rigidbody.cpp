#include <latren/physics/component/rigidbody.h>
#include <latren/physics/utils.h>
#include <latren/entity/entity.h>
#include <latren/graphics/component/meshrenderer.h>

using namespace Physics;

// really not feeling it right now, just copied from here
// https://pybullet.org/Bullet/phpBB3/viewtopic.php?t=7802
// (this even comes with 16 bit optimizations!!!)
std::shared_ptr<btBvhTriangleMeshShape> RigidBody::CreateMeshCollider() {
    meshData_ = std::make_shared<btTriangleIndexVertexArray>();
    const std::vector<std::shared_ptr<Mesh>>& meshes = parent.GetComponent<MeshRenderer>().meshes;
    for (int meshIndex = 0; meshIndex < meshes.size(); meshIndex++) {
        if (colliderMeshes.size() > 0 && std::find(colliderMeshes.begin(), colliderMeshes.end(), meshIndex) == colliderMeshes.end())
            continue;
        const std::shared_ptr<Mesh>& m = meshes.at(meshIndex);
        btIndexedMesh tempMesh;
        meshData_->addIndexedMesh(tempMesh, PHY_FLOAT);

        btIndexedMesh& mesh = meshData_->getIndexedMeshArray()[meshIndex];

        size_t numIndices = m->indices.size();
        mesh.m_numTriangles = (int) numIndices / 3;
        
        unsigned char* indices = nullptr;
        unsigned char* vertices = nullptr;
        if (numIndices < std::numeric_limits<int16_t>::max()) {
            // we can use 16-bit indices
            indices = new unsigned char[sizeof(int16_t) * (size_t) numIndices];
            mesh.m_indexType = PHY_SHORT;
            mesh.m_triangleIndexStride = 3 * sizeof(int16_t);
        }
        else {
            // we need 32-bit indices
            indices = new unsigned char[sizeof(int32_t) * (size_t) numIndices];
            mesh.m_indexType = PHY_INTEGER;
            mesh.m_triangleIndexStride = 3 * sizeof(int32_t);
        }

        mesh.m_numVertices = (int) m->vertices.size();
        vertices = new unsigned char[sizeof(btScalar) * (size_t) mesh.m_numVertices];
        mesh.m_vertexStride = 3 * sizeof(btScalar);

        // copy vertices into mesh
        btScalar* scalarVertices = reinterpret_cast<btScalar*>(vertices);
        for (int32_t i = 0; i < mesh.m_numVertices; i++) {
            scalarVertices[i] = m->vertices[i];
        }
        // copy indices into mesh
        if (numIndices < std::numeric_limits<int16_t>::max()) {
            // 16-bit case
            int16_t* indices_16 = reinterpret_cast<int16_t*>(indices);
            for (int32_t i = 0; i < numIndices; i++) {
                indices_16[i] = (int16_t) m->indices[i];
            }
        }
        else {
            // 32-bit case
            int32_t* indices_32 = reinterpret_cast<int32_t*>(indices);
            for (int32_t i = 0; i < numIndices; i++) {
                indices_32[i] = m->indices[i];
            }
        }
        mesh.m_vertexBase = vertices;
        mesh.m_triangleIndexBase = indices;
    }
    std::shared_ptr<btBvhTriangleMeshShape> colliderShape = std::make_shared<btBvhTriangleMeshShape>(meshData_.get(), true);
    return colliderShape;
}

void RigidBody::Start() {
    Transform& t = parent.GetTransform();
    btTransform transform;
    transform.setIdentity();
    meshData_ = nullptr;
    rigidBody = nullptr;

    if (collider == nullptr) {
        switch (colliderFrom) {
            case ColliderConstructor::TRANSFORM:
                collider = std::make_shared<btBoxShape>(btVector3(.5f, .5f, .5f));
                break;
            case ColliderConstructor::AABB:
                collider = std::make_shared<btBoxShape>(Physics::GLMVectorToBtVector3(parent.GetComponent<MeshRenderer>().GetAABB().extents));
                colliderOriginOffset_ = parent.GetComponent<MeshRenderer>().GetAABB().center * t.size;
                transform.setOrigin(Physics::GLMVectorToBtVector3(t.rotation * colliderOriginOffset_));
                break;
            case ColliderConstructor::MESH:
                collider = CreateMeshCollider();
                break;
        }
    }
    collider->setLocalScaling(collider->getLocalScaling() * Physics::GLMVectorToBtVector3(t.size));
    btVector3 localInertia(0, 0, 0);
    if (mass != 0.0f)
        collider->calculateLocalInertia(mass, localInertia);

    transform.setOrigin(transform.getOrigin() + Physics::GLMVectorToBtVector3(t.position));
    transform.setRotation(Physics::GLMQuatToBtQuat(t.rotation));

    if (rigidBody == nullptr) {
        btDefaultMotionState* motionState = new btDefaultMotionState(transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, collider.get(), localInertia);
        rbInfo.m_restitution = 0.0f;
        rigidBody = std::make_shared<RAIIBtCollisionObject<btRigidBody>>(rbInfo);
    }

    EnableDebugVisualization(enableDebugVisualization_);
    EnableRotation(enableRotation_);
    rigidBody->Get()->setRestitution(0.0f);
    rigidBody->Get()->setDamping(0.0f, 1.0f);
    rigidBody->Get()->setCollisionFlags(rigidBody->Get()->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
    if (doesMassAffectGravity)
        rigidBody->Get()->setGravity(rigidBody->Get()->getGravity() * mass);
    if (disableCollisions)
        rigidBody->Get()->setCollisionFlags(rigidBody->Get()->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    if (overwriteTransform)
        UpdateTransform();
}

void RigidBody::UpdateTransform() {
    Transform& t = parent.GetTransform();
    btTransform transform;
    rigidBody->Get()->getMotionState()->getWorldTransform(transform);
    glm::quat rot = Physics::BtQuatToGLMQuat(transform.getRotation());

    t.position = Physics::BtVectorToGLMVector3(transform.getOrigin()) - rot * colliderOriginOffset_;
    t.rotation = rot;
}

void RigidBody::CopyTransform() {
    Transform& t = parent.GetTransform();
    btTransform transform;
    btMotionState* ms = rigidBody->Get()->getMotionState();
    ms->getWorldTransform(transform);
    
    transform.setOrigin(Physics::GLMVectorToBtVector3(t.position + t.rotation * colliderOriginOffset_));
    transform.setRotation(t.btGetRotation());

    ms->setWorldTransform(transform);
    rigidBody->Get()->setMotionState(ms);
}

void RigidBody::Update() {
    if (interpolate) {
        if (overwriteTransform)
            UpdateTransform();
        else
            CopyTransform();
    }
}

void RigidBody::FixedUpdate() {
    if (!interpolate) {
        if (overwriteTransform)
            UpdateTransform();
        else
            CopyTransform();
    }
    //rigidBody->getCollisionShape()->setLocalScaling(btVector3(t->size.x, t->size.y, t->size.z));
}

void RigidBody::SetPos(const glm::vec3& pos) {
    btTransform transform;
    btMotionState* ms = rigidBody->Get()->getMotionState();
    ms->getWorldTransform(transform);
    transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
    ms->setWorldTransform(transform);
    rigidBody->Get()->setMotionState(ms);
    parent.GetTransform().position = pos;
}

void RigidBody::EnableDebugVisualization(bool enabled) {
    enableDebugVisualization_ = enabled;
    if (rigidBody == nullptr)
        return;
    
    if (enableDebugVisualization_)
        rigidBody->Get()->setCollisionFlags(rigidBody->Get()->getCollisionFlags() & ~btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT);
    else
        rigidBody->Get()->setCollisionFlags(rigidBody->Get()->getCollisionFlags() | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT);
}

void RigidBody::EnableRotation(bool enabled) {
    enableRotation_ = enabled;
    if (rigidBody == nullptr)
        return;
    rigidBody->Get()->setAngularFactor(1.0f * enableRotation_);
}


// serialization
#include <latren/io/serializer.h>
#include <latren/io/serializetypes.h>

JSON_SERIALIZE_ENUM(ColliderConstructor);

// TODO: create colliders from scratch
JSON_SERIALIZE_TYPES([](Serializer::SerializationArgs& args, const nlohmann::json& j) {
    btCollisionShape* collider = nullptr;
    args.Return(collider);
    return true;
}, btCollisionShape*);