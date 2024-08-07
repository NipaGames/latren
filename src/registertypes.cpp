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

void ComponentSerialization::RegisterCoreComponents() {
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

#include <latren/io/genericdeserializers.h>
#include <latren/graphics/shader.h>
#include <latren/graphics/mesh.h>
#include <latren/graphics/renderer.h>
#include <latren/terrain/plane.h>
#include <latren/systems.h>

void ComponentSerialization::RegisterCoreDeserializers() {
    // JSON serializers
    Serialization::AssignJSONDeserializer<int>(DeserializeJSONNumber<int>);
    Serialization::AssignJSONDeserializer<float>(DeserializeJSONNumber<float>);
    Serialization::AssignJSONDeserializer<glm::vec2, glm::ivec2>(DeserializeJSONVector<2>);
    Serialization::AssignJSONDeserializer<glm::vec3, glm::ivec3>(DeserializeJSONVector<3>);
    Serialization::AssignJSONDeserializer<Quaternion>([](Serialization::DeserializationContext& args, const nlohmann::json& j) {
        glm::vec3 eulers;
        Serialization::DeserializationContext vecArgs;
        vecArgs.type = Serialization::DeserializationContext::DeserializerType::ANY_POINTER;
        vecArgs.ptr = &eulers;
        if (!DeserializeJSONVector<3>(vecArgs, j))
            return false;
        args.Return(Quaternion(glm::radians(eulers)));
        return true;
    });
    Serialization::AssignJSONDeserializer<std::string>([](Serialization::DeserializationContext& args, const nlohmann::json& j) {
        if (j.is_string()) {
            args.Return(j.get<std::string>());
            return true;
        }
        return false;
    });
    Serialization::AssignJSONDeserializer<bool>([](Serialization::DeserializationContext& args, const nlohmann::json& j) {
        if (!j.is_boolean())
            return false;
        args.Return((bool) j);
        return true;
    });
    Serialization::AssignJSONDeserializer<Shaders::ShaderID>([](Serialization::DeserializationContext& args, const nlohmann::json& j) {
        if (!j.is_string())
            return false;
        auto s = magic_enum::enum_cast<Shaders::ShaderID>((std::string) j);
        if (!s.has_value())
            return false;
        args.Return(s.value());
        return true;
    });
    Serialization::AssignJSONDeserializer<std::shared_ptr<Material>>([](Serialization::DeserializationContext& args, const nlohmann::json& j) {
        if (!j.is_string())
            return false;
        args.Return(Systems::GetRenderer().GetMaterial(j));
        return true;
    });

    // CFG serializers
    using namespace CFG;
    Serialization::AssignCFGDeserializer<float>([](Serialization::DeserializationContext& args, const ICFGField* field) {
        if (!IsValidType(field->type, CFGFieldType::FLOAT))
            return false;
        args.Return(field->GetValue<float>());
        return true;
    });
    Serialization::AssignCFGDeserializer<int, bool>([](Serialization::DeserializationContext& args, const ICFGField* field) {
        if (!IsValidType(field->type, CFGFieldType::INTEGER))
            return false;
        args.Return(field->GetValue<int>());
        return true;
    });

    Serialization::AssignCFGDeserializer<glm::vec2>(DeserializeCFGVectorAuto<2, float>);
    Serialization::AssignCFGDeserializer<glm::ivec2>(DeserializeCFGVectorAuto<2, int>);

    Serialization::AssignJSONDeserializer<AudioBufferHandle>([](Serialization::DeserializationContext& args, const nlohmann::json& j) {
        if (!j.is_string())
            return false;
        args.Return(Systems::GetResources().GetAudioManager()->Get(j));
        return true;
    });
    Serialization::AssignJSONEnumDeserializer<AudioSourceRelativeTo>();

    Serialization::AssignJSONEnumDeserializer<RenderPass::Enum>();
    Serialization::AssignJSONDeserializer<std::shared_ptr<Mesh>>([](Serialization::DeserializationContext& args, const nlohmann::json& j) {
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
                if (!Serialization::SetJSONPointerValue(&tiling, j["tiling"])) {
                    return false;
                }
            }
            if (j.contains("variation")) {
                if (!Serialization::SetJSONPointerValue(&variation, j["variation"])) {
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

    Serialization::AssignJSONEnumDeserializer<Physics::ColliderConstructor>();

    // TODO: create colliders from scratch
    Serialization::AssignJSONDeserializer<btCollisionShape*>([](Serialization::DeserializationContext& args, const nlohmann::json& j) {
        btCollisionShape* collider = nullptr;
        args.Return(collider);
        return true;
    });
}