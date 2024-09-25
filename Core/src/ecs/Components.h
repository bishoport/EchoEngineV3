#pragma once
#include <entt.hpp>

#include "../core/UUID.h"
#include "../core/model/Mesh.h"
#include "../core/model/Model.h"
#include "../core/model/skeletal/animation.h"
#include "../tools/cameras/Camera.h"


namespace libCore
{
    struct IDComponent {
        UUID ID;
        bool markToDelete = false;

        IDComponent() = default;
        IDComponent(const IDComponent&) = default;
        IDComponent(UUID id)
            : ID(id) {}
    };
    struct TagComponent {
        std::string Tag;

        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent(const std::string& tag)
            : Tag(tag) {}
    };
    struct ParentComponent {
        entt::entity parent = entt::null;
    };
    struct ChildComponent {
        std::vector<entt::entity> children;
    };
    struct MeshComponent
    {
        bool renderable = true; //<- no est� serializada, hay que agregarla

        Ref<Mesh> mesh = CreateRef<Mesh>();
        std::vector<glm::mat4> instanceMatrices;
        Ref<Model> originalModel = nullptr; // Referencia al modelo original
        bool isInstance = false; // Indica si es una instancia

        MeshComponent() = default;
        MeshComponent(const Ref<Mesh>& mesh, const Ref<Model>& originalModel = nullptr, bool isInstance = false) : mesh(mesh), originalModel(originalModel), isInstance(isInstance) {}
    };
    struct AABBComponent
    {
        Ref<AABB> aabb = CreateRef<AABB>();
    };
    struct MaterialComponent
    {
        Ref<Material> material;
    };
    struct TransformComponent {

        Ref<Transform> transform = CreateRef<Transform>();
        glm::mat4 accumulatedTransform = glm::mat4(1.0f); // Transformaci�n acumulada

        // Funciones existentes para obtener y establecer transformaciones globales
        glm::mat4 getGlobalTransform(entt::entity entity, entt::registry& registry) const {
            glm::mat4 globalTransform = transform->getLocalModelMatrix();
            if (registry.has<ParentComponent>(entity)) {
                entt::entity parentEntity = registry.get<ParentComponent>(entity).parent;
                if (registry.valid(parentEntity)) {
                    globalTransform = registry.get<TransformComponent>(parentEntity).accumulatedTransform * globalTransform;
                }
            }
            return globalTransform;
        }

        void setTransformFromGlobal(const glm::mat4& globalTransform, entt::entity entity, entt::registry& registry) {
            glm::mat4 parentGlobalTransform = glm::mat4(1.0f);
            if (registry.has<ParentComponent>(entity)) {
                entt::entity parentEntity = registry.get<ParentComponent>(entity).parent;
                if (registry.valid(parentEntity)) {
                    parentGlobalTransform = registry.get<TransformComponent>(parentEntity).accumulatedTransform;
                }
            }
            glm::mat4 newLocalTransform = glm::inverse(parentGlobalTransform) * globalTransform;
            transform->setMatrix(newLocalTransform);
        }

        // Funciones adicionales para acceder y modificar la posici�n, rotaci�n y escala desde Lua
        glm::vec3 GetPosition() const {
            return transform->GetPosition();
        }

        void SetPosition(const glm::vec3& position) {
            transform->SetPosition(position);
        }

        glm::quat GetRotation() const {
            return transform->GetRotation();
        }

        void SetRotation(const glm::quat& rotation) {
            transform->SetRotation(rotation);
        }

        void SetRotationEuler(const glm::vec3& eulerAngles) {
            transform->SetEulerAngles(eulerAngles);
        }

        glm::vec3 GetEulerAngles() const {
            return transform->GetEulerAngles();
        }

        glm::vec3 GetScale() const {
            return transform->GetScale();
        }

        void SetScale(const glm::vec3& scale) {
            transform->SetScale(scale);
        }

        // Funciones adicionales de utilidad
        glm::mat4 GetLocalModelMatrix() const {
            return transform->getLocalModelMatrix();
        }
    };
    struct CameraComponent
    {
        Ref<Camera> camera = nullptr;
    };
    struct CreatedInRunTimeComponent
    {
        std::string prueba = "";
    };
    struct AnimationComponent
    {
        // Mapa de animaciones: el nombre de la animaci�n como clave
        std::unordered_map<std::string, Ref<Animation>> animations;

        // Par�metros para controlar la animaci�n
        std::string currentAnimation;    // Nombre de la animaci�n actual
        float animationTime = 0.0f;      // Tiempo transcurrido en la animaci�n
        float playbackSpeed = 1.0f;      // Velocidad de reproducci�n
        bool isPlaying = true;           // Control de reproducci�n
        Ref<Model> model = nullptr;      // Referencia al modelo animado

        // Transformaciones finales de huesos
        std::vector<glm::mat4> finalBoneMatrices;

        // Constructor
        AnimationComponent() = default;
        AnimationComponent(const Ref<Model>& model) : model(model) {
            finalBoneMatrices.resize(100, glm::mat4(1.0f)); // Inicializa las matrices de huesos
        }

        // M�todo para agregar una animaci�n
        void AddAnimation(const std::string& name, const std::string& filePath) {
            animations[name] = CreateRef<Animation>(filePath, model);
        }

        // M�todo para cambiar la animaci�n actual
        void SetCurrentAnimation(const std::string& name) {
            if (animations.find(name) != animations.end()) {
                currentAnimation = name;
                animationTime = 0.0f;  // Reinicia el tiempo de la animaci�n
            }
            else {
                std::cerr << "Error: Animaci�n '" << name << "' no encontrada." << std::endl;
            }
        }

        // Obtener la animaci�n actual
        Ref<Animation> GetCurrentAnimation() {
            if (animations.find(currentAnimation) != animations.end()) {
                return animations[currentAnimation];
            }
            return nullptr;
        }

        // Actualizar el tiempo de animaci�n (se llama en cada frame)
        void Update(float deltaTime) {
            if (!isPlaying || currentAnimation.empty()) return;

            animationTime += deltaTime * playbackSpeed;

            // Obtener la animaci�n actual para controlar la duraci�n
            auto anim = GetCurrentAnimation();
            if (anim) {
                float duration = anim->GetDuration();
                if (animationTime > duration) {
                    animationTime = fmod(animationTime, duration);  // Reinicia la animaci�n al final
                }

                // Actualizar la animaci�n antes de calcular la transformaci�n de los huesos
                anim->Update(deltaTime);

                // Llamar a la funci�n para calcular la transformaci�n de los huesos
                CalculateBoneTransform(&anim->GetRootNode(), glm::mat4(1.0f));
            }
        }

        // Funci�n para calcular las transformaciones de los huesos
        void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform) {
            std::string nodeName = node->name;
            glm::mat4 nodeTransform = node->transformation;

            // Buscar el hueso en la animaci�n actual
            auto anim = GetCurrentAnimation();
            if (anim) {
                Bone* bone = anim->FindBone(nodeName);
                if (bone) {
                    bone->Update(animationTime);
                    nodeTransform = bone->GetLocalTransform();
                }
            }

            glm::mat4 globalTransformation = parentTransform * nodeTransform;

            // Actualizar las matrices de huesos si el nodo es un hueso
            auto boneInfoMap = anim->GetBoneIDMap();
            if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
                int index = boneInfoMap[nodeName].id;
                glm::mat4 offset = boneInfoMap[nodeName].offset;
                finalBoneMatrices[index] = globalTransformation * offset;
            }

            // Procesar los hijos recursivamente
            for (int i = 0; i < node->childrenCount; i++) {
                CalculateBoneTransform(&node->children[i], globalTransformation);
            }
        }

        // Obtener las matrices finales de huesos
        std::vector<glm::mat4> GetFinalBoneMatrices() {
            return finalBoneMatrices;
        }
    };


}