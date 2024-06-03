#include <latren/game.h>
#include <latren/ec/component.h>
#include <latren/ec/serialization.h>

#include <latren/ec/transform.h>
#include <latren/audio/component/audiosourcecomponent.h>
#include <latren/graphics/component/meshrenderer.h>
#include <latren/graphics/component/billboard.h>
#include <latren/graphics/component/light.h>
#include <latren/physics/component/rigidbody.h>
#include <latren/ui/component/containercomponent.h>
#include <latren/ui/component/listcomponent.h>
#include <latren/ui/component/textcomponent.h>
#include <latren/ui/component/textinputcomponent.h>
#include <latren/ui/component/uicomponent.h>

void Game::RegisterComponents() {
    LATREN_REGISTER_COMPONENT(Transform);

    LATREN_REGISTER_COMPONENT(AudioSourceComponent);

    LATREN_REGISTER_COMPONENT(MeshRenderer);
    LATREN_REGISTER_COMPONENT(BillboardRenderer);
    LATREN_REGISTER_COMPONENT(Lights::PointLight);
    LATREN_REGISTER_COMPONENT(Lights::DirectionalLight);
    LATREN_REGISTER_COMPONENT(Lights::DirectionalLightPlane);
    LATREN_REGISTER_COMPONENT(Lights::Spotlight);

    LATREN_REGISTER_COMPONENT(Physics::RigidBody);

    LATREN_REGISTER_COMPONENT(UI::ContainerComponent);
    LATREN_REGISTER_COMPONENT(UI::ListComponent);
    LATREN_REGISTER_COMPONENT(UI::TextListComponent);
    LATREN_REGISTER_COMPONENT(UI::TextComponent);
    LATREN_REGISTER_COMPONENT(UI::TextInputComponent);
    LATREN_REGISTER_COMPONENT(UI::UIComponent);
}

#include <latren/io/serializer.h>
#include <latren/io/files/cfg.h>
#include <latren/graphics/shader.h>
#include <latren/graphics/mesh.h>
#include <latren/graphics/renderer.h>
#include <latren/terrain/plane.h>
#include <latren/systems.h>

template <typename T>
bool DeserializeJSONNumber(Serializer::DeserializationArgs& args, const nlohmann::json& j) {
    if (!j.is_number())
        return false;
    args.Return<T>(j);
    return true;
}

template <size_t S>
bool DeserializeJSONVector(Serializer::DeserializationArgs& args, const nlohmann::json& j) {
    typedef glm::vec<S, float> Vector;
    Vector vec;
    if (j.is_number()) vec = Vector(j);
    else if (j.is_array() && j.size() == S) {
        for (int i = 0; i < S; i++) {
            if (!j[i].is_number()) return false;
            vec[i] = j[i];
        }
    }
    else return false;
    args.Return(vec);
    return true;
}

template <size_t S, typename T>
bool DeserializeCFGVector(Serializer::DeserializationArgs& args, const CFG::ICFGField* field, CFG::CFGFieldType type) {
    if (field->type != CFG::CFGFieldType::STRUCT && field->type != CFG::CFGFieldType::ARRAY)
        return false;
    const CFG::CFGObject* obj = static_cast<const CFG::CFGObject*>(field);
    const auto& items = obj->GetItems();
    if (items.size() != S)
        return false;
    glm::vec<S, T> vec;
    for (int i = 0; i < S; i++) {
        const CFG::ICFGField* child = items.at(i);
        if (!IsValidType(child->type, type))
            return false;
        vec[i] = child->GetValue<T>();
    }
    args.Return(vec);
    return true;
}

template <size_t S, typename T, CFG::CFGFieldType F>
bool DeserializeCFGVector(Serializer::DeserializationArgs& args, const CFG::ICFGField* field) {
    return DeserializeCFGVector<S, T>(args, field, F);
}

template <size_t S, typename T>
bool DeserializeCFGVectorAuto(Serializer::DeserializationArgs& args, const CFG::ICFGField* field) {
    size_t typeHash = typeid(T).hash_code();
    if (CFG::CFG_TYPES_.find(typeHash) == CFG::CFG_TYPES_.end())
        return false;
    return DeserializeCFGVector<S, T>(args, field, CFG::CFG_TYPES_.at(typeHash).front());
}

void Game::RegisterDeserializers() {
    // JSON serializers
    Serializer::AddJSONDeserializer<int>(DeserializeJSONNumber<int>);
    Serializer::AddJSONDeserializer<float>(DeserializeJSONNumber<float>);
    Serializer::AddJSONDeserializer<glm::vec2, glm::ivec2>(DeserializeJSONVector<2>);
    Serializer::AddJSONDeserializer<glm::vec3, glm::ivec3>(DeserializeJSONVector<3>);
    Serializer::AddJSONDeserializer<glm::quat>([](Serializer::DeserializationArgs& args, const nlohmann::json& j) {
        glm::vec3 eulers;
        Serializer::DeserializationArgs vecArgs(Serializer::DeserializerType::ANY_POINTER);
        vecArgs.ptr = &eulers;
        if (!DeserializeJSONVector<3>(vecArgs, j))
            return false;
        args.Return(glm::quat(glm::radians(eulers)));
        return true;
    });
    Serializer::AddJSONDeserializer<std::string>([](Serializer::DeserializationArgs& args, const nlohmann::json& j) {
        if (j.is_string()) {
            args.Return((std::string) j);
            return true;
        }
        return false;
    });
    Serializer::AddJSONDeserializer<bool>([](Serializer::DeserializationArgs& args, const nlohmann::json& j) {
        if (!j.is_boolean())
            return false;
        args.Return((bool) j);
        return true;
    });
    Serializer::AddJSONDeserializer<Shaders::ShaderID>([](Serializer::DeserializationArgs& args, const nlohmann::json& j) {
        if (!j.is_string())
            return false;
        auto s = magic_enum::enum_cast<Shaders::ShaderID>((std::string) j);
        if (!s.has_value())
            return false;
        args.Return(s.value());
        return true;
    });
    Serializer::AddJSONDeserializer<std::shared_ptr<Material>>([](Serializer::DeserializationArgs& args, const nlohmann::json& j) {
        if (!j.is_string())
            return false;
        args.Return(Systems::GetRenderer().GetMaterial(j));
        return true;
    });

    // CFG serializers
    using namespace CFG;
    Serializer::AddCFGDeserializer<float>([](Serializer::DeserializationArgs& args, const ICFGField* field) {
        if (!IsValidType(field->type, CFGFieldType::FLOAT))
            return false;
        args.Return(field->GetValue<float>());
        return true;
    });
    Serializer::AddCFGDeserializer<int, bool>([](Serializer::DeserializationArgs& args, const ICFGField* field) {
        if (!IsValidType(field->type, CFGFieldType::INTEGER))
            return false;
        args.Return(field->GetValue<int>());
        return true;
    });

    Serializer::AddCFGDeserializer<glm::vec2>(DeserializeCFGVectorAuto<2, float>);
    Serializer::AddCFGDeserializer<glm::ivec2>(DeserializeCFGVectorAuto<2, int>);

    Serializer::AddJSONDeserializer<AudioBufferHandle>([](Serializer::DeserializationArgs& args, const nlohmann::json& j) {
        if (!j.is_string())
            return false;
        args.Return(Systems::GetResources().audioManager.Get(j));
        return true;
    });
    Serializer::AddJSONEnumDeserializer<AudioSourceRelativeTo>();

    Serializer::AddJSONEnumDeserializer<RenderPass::Enum>();
    Serializer::AddJSONDeserializer<std::shared_ptr<Mesh>>([](Serializer::DeserializationArgs& args, const nlohmann::json& j) {
        if (!j.is_object())
            return false;
        if (!j.contains("type") || !j["type"].is_string())
            return false;
        std::string type = j["type"];
        if (!j.contains("material") || !j["material"].is_string())
            return false;
        std::string material = j["material"];
        std::shared_ptr<Mesh> mesh = nullptr;

        if (type == "CUBE") {
            mesh = Meshes::CreateMeshInstance(Meshes::CUBE);
        }
        else if (type == "TERRAIN") {
            glm::vec2 tiling = glm::vec2(1.0f);
            float variation = 0.0f;

            if (j.contains("tiling")) {
                if (!Serializer::SetJSONPointerValue(&tiling, j["tiling"])) {
                    return false;
                }
            }
            if (j.contains("variation")) {
                if (!Serializer::SetJSONPointerValue(&variation, j["variation"])) {
                    return false;
                }
            }
            
            std::shared_ptr<Plane> plane = std::make_shared<Plane>(tiling);
            plane->heightVariation = variation;
            plane->GenerateVertices();
            mesh = plane;
        }

        if (mesh == nullptr)
            return false;
        mesh->material = Systems::GetRenderer().GetMaterial(material);
        mesh->GenerateVAO();
        args.Return(mesh);
        return true;
    });

    Serializer::AddJSONEnumDeserializer<Physics::ColliderConstructor>();

    // TODO: create colliders from scratch
    Serializer::AddJSONDeserializer<btCollisionShape*>([](Serializer::DeserializationArgs& args, const nlohmann::json& j) {
        btCollisionShape* collider = nullptr;
        args.Return(collider);
        return true;
    });
}