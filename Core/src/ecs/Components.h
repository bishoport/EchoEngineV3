#pragma once
#include <entt.hpp>

#include "../core/UUID.h"
#include "../core/model/Mesh.h"
#include "../core/model/Model.h"
#include "../tools/cameras/Camera.h"

#include "../core/model/skeletal/animator.h"
#include "../core/model/skeletal/animation.h"

namespace libCore
{
    //COMPONENTS
    struct IDComponent {
        UUID ID;  // Ahora es de 24 bits
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
    //struct MeshComponent
    //{
    //    bool renderable = true; // <- no est� serializada, hay que agregarla

    //    Ref<Mesh> mesh = CreateRef<Mesh>();
    //    std::vector<glm::mat4> instanceMatrices;
    //    Ref<Model> originalModel = nullptr; // Referencia al modelo original
    //    bool isInstance = false; // Indica si es una instancia

    //    MeshComponent() = default;
    //    MeshComponent(const Ref<Mesh>& mesh, const Ref<Model>& originalModel = nullptr, bool isInstance = false)
    //        : mesh(mesh), originalModel(originalModel), isInstance(isInstance) {}

    //    // M�todo para clonar una malla
    //    static Ref<Mesh> CloneMesh(const Ref<Mesh>& meshToClone)
    //    {
    //        // Crear una nueva instancia del objeto `Mesh` a partir de la copia
    //        return CreateRef<Mesh>(*meshToClone);
    //    }
    //};


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

        // Funci�n para obtener la transformaci�n global del componente
        glm::mat4 getGlobalTransform(entt::entity entity, entt::registry& registry) {
            if (transform->isTransformDirty()) {
                transform->computeModelMatrix();
            }

            glm::mat4 globalTransform = transform->modelMatrix;
            if (registry.has<ParentComponent>(entity)) {
                entt::entity parentEntity = registry.get<ParentComponent>(entity).parent;
                if (registry.valid(parentEntity)) {
                    globalTransform = registry.get<TransformComponent>(parentEntity).getGlobalTransform(parentEntity, registry) * globalTransform;
                }
            }
            accumulatedTransform = globalTransform;
            return accumulatedTransform;
        }

        // Establece la transformaci�n local del componente a partir de una matriz global
        void setTransformFromGlobal(const glm::mat4& globalTransform, entt::entity entity, entt::registry& registry) {
            glm::mat4 parentGlobalTransform = glm::mat4(1.0f);
            if (registry.has<ParentComponent>(entity)) {
                entt::entity parentEntity = registry.get<ParentComponent>(entity).parent;
                if (registry.valid(parentEntity)) {
                    parentGlobalTransform = registry.get<TransformComponent>(parentEntity).getGlobalTransform(parentEntity, registry);
                }
            }
            glm::mat4 newLocalTransform = glm::inverse(parentGlobalTransform) * globalTransform;
            transform->setMatrix(newLocalTransform);
            MarkDirty();
        }

        // Funciones adicionales para acceder y modificar la posici�n, rotaci�n y escala desde Lua
        glm::vec3 GetPosition() const {
            return transform->GetPosition();
        }

        void SetPosition(const glm::vec3& position) {
            transform->SetPosition(position);
            MarkDirty();
        }

        glm::quat GetRotation() const {
            return transform->GetRotation();
        }

        void SetRotation(const glm::quat& rotation) {
            transform->SetRotation(rotation);
            MarkDirty();
        }

        void SetRotationEuler(const glm::vec3& eulerAngles) {
            transform->SetEulerAngles(eulerAngles);
            MarkDirty();
        }

        glm::vec3 GetEulerAngles() const {
            return transform->GetEulerAngles();
        }

        glm::vec3 GetScale() const {
            return transform->GetScale();
        }

        void SetScale(const glm::vec3& scale) {
            transform->SetScale(scale);
            MarkDirty();
        }

        // Funci�n para obtener la matriz del modelo local
        glm::mat4 GetLocalModelMatrix() const {
            if (transform->isTransformDirty()) {
                transform->computeModelMatrix();
            }
            return transform->getLocalModelMatrix();
        }

        // Marcar la transformaci�n como sucia
        void MarkDirty() {
            transform->isDirty = true;
        }

        // Funci�n para recalcular la transformaci�n si es necesario
        void UpdateIfNeeded() {
            if (transform->isTransformDirty()) {
                transform->computeModelMatrix();
            }
        }
    };



    //struct TransformComponent {

    //    Ref<Transform> transform = CreateRef<Transform>();
    //    glm::mat4 accumulatedTransform = glm::mat4(1.0f); // Transformaci�n acumulada

    //    // Funciones existentes para obtener y establecer transformaciones globales
    //    glm::mat4 getGlobalTransform(entt::entity entity, entt::registry& registry) const {
    //        glm::mat4 globalTransform = transform->getLocalModelMatrix();
    //        if (registry.has<ParentComponent>(entity)) {
    //            entt::entity parentEntity = registry.get<ParentComponent>(entity).parent;
    //            if (registry.valid(parentEntity)) {
    //                globalTransform = registry.get<TransformComponent>(parentEntity).accumulatedTransform * globalTransform;
    //            }
    //        }
    //        return globalTransform;
    //    }

    //    void setTransformFromGlobal(const glm::mat4& globalTransform, entt::entity entity, entt::registry& registry) {
    //        glm::mat4 parentGlobalTransform = glm::mat4(1.0f);
    //        if (registry.has<ParentComponent>(entity)) {
    //            entt::entity parentEntity = registry.get<ParentComponent>(entity).parent;
    //            if (registry.valid(parentEntity)) {
    //                parentGlobalTransform = registry.get<TransformComponent>(parentEntity).accumulatedTransform;
    //            }
    //        }
    //        glm::mat4 newLocalTransform = glm::inverse(parentGlobalTransform) * globalTransform;
    //        transform->setMatrix(newLocalTransform);
    //    }

    //    // Funciones adicionales para acceder y modificar la posici�n, rotaci�n y escala desde Lua
    //    glm::vec3 GetPosition() const {
    //        return transform->GetPosition();
    //    }

    //    void SetPosition(const glm::vec3& position) {
    //        transform->SetPosition(position);
    //    }

    //    glm::quat GetRotation() const {
    //        return transform->GetRotation();
    //    }

    //    void SetRotation(const glm::quat& rotation) {
    //        transform->SetRotation(rotation);
    //    }

    //    void SetRotationEuler(const glm::vec3& eulerAngles) {
    //        transform->SetEulerAngles(eulerAngles);
    //    }

    //    glm::vec3 GetEulerAngles() const {
    //        return transform->GetEulerAngles();
    //    }

    //    glm::vec3 GetScale() const {
    //        return transform->GetScale();
    //    }

    //    void SetScale(const glm::vec3& scale) {
    //        transform->SetScale(scale);
    //    }

    //    // Funciones adicionales de utilidad
    //    glm::mat4 GetLocalModelMatrix() const {
    //        return transform->getLocalModelMatrix();
    //    }
    //};
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
        float boneScaleFactor = 1.0f;

        // Mapa de animaciones: el nombre de la animaci�n como clave
        std::unordered_map<std::string, Ref<Animation>> animations;

        // Par�metros para controlar la animaci�n
        std::string currentAnimation;    // Nombre de la animaci�n actual
        float animationTime = 0.0f;      // Tiempo transcurrido en la animaci�n
        float playbackSpeed = 1.0f;      // Velocidad de reproducci�n
        bool isPlaying = true;           // Control de reproducci�n
        Ref<libCore::Model> model = nullptr;      // Referencia al modelo animado

        Ref<Animator> animator;  // Animador para gestionar las animaciones

        // Constructor
        AnimationComponent() = default;
        AnimationComponent(const Ref<Model>& model) : model(model) {
            animator = CreateRef<Animator>();  // Inicializa el animador sin animaci�n
        }

        // M�todo para agregar una animaci�n
        void AddAnimation(const std::string& name, const std::string& filePath) {
            if (model != nullptr) {
                // Obtener el directorio actual de trabajo (ruta de la soluci�n)
                std::filesystem::path solutionPath = std::filesystem::current_path();
                std::filesystem::path relativePath("assets/" + filePath);

                // Concatenar la ruta de la soluci�n con la ruta relativa
                std::filesystem::path fullPath = solutionPath / relativePath;

                // Convertir la ruta a string
                std::string fullPathStr = fullPath.string();

                // Imprimir la ruta completa para depuraci�n
                std::cout << "Cargando animaci�n desde: " << fullPathStr << std::endl;

                // Usar la ruta completa para cargar la animaci�n
                animations[name] = CreateRef<Animation>(fullPathStr, model);

                if (animations[name]) {
                    std::cout << "Animaci�n '" << name << "' agregada correctamente." << std::endl;
                }
                else {
                    std::cerr << "Error: La animaci�n '" << name << "' no se pudo cargar desde el archivo." << std::endl;
                }
            }
            else {
                std::cerr << "Error: El modelo no est� asignado al componente de animaci�n." << std::endl;
            }
        }

        // M�todo para cambiar la animaci�n actual
        void SetCurrentAnimation(const std::string& name) {
            std::cout << "Intentando establecer la animaci�n: " << name << std::endl;

            if (animations.find(name) != animations.end()) {
                currentAnimation = name;
                animationTime = 0.0f;  // Reinicia el tiempo de la animaci�n
                
                if (animator) {
                    std::cout << "Estableciendo la animaci�n actual en el animador: " << name << std::endl;
                    animator->SetAnimation(animations[name]);  // Usa SetAnimation en el animador

                    if (!animator->HasAnimation()) {
                        std::cerr << "Advertencia: El animador no tiene ninguna animaci�n asignada." << std::endl;
                    }
                }
                else {
                    std::cerr << "Error: El animador no est� inicializado." << std::endl;
                }
            }
            else {
                std::cerr << "Error: Animaci�n '" << name << "' no encontrada." << std::endl;
            }
        }

        //// Actualizar el tiempo de animaci�n (se llama en cada frame)
        //void Update(float deltaTime) {
        //    if (!isPlaying || currentAnimation.empty()) {
        //        return;
        //    }
        //    animationTime += deltaTime * playbackSpeed;
        //    if (animator) {
        //        animator->UpdateAnimation(deltaTime);
        //    }
        //    else {
        //        std::cerr << "Error: El animador no est� inicializado." << std::endl;
        //    }
        //}

        // Actualizar el tiempo de animaci�n (se llama en cada frame)
        void Update(float deltaTime) {
            if (!isPlaying || currentAnimation.empty()) {
                return;
            }
            animationTime += deltaTime * playbackSpeed;
            if (animator) {
                animator->UpdateAnimation(deltaTime);

                // Aqu� actualizamos las matrices finales de los huesos para cada frame
                auto finalBoneMatrices = animator->GetFinalBoneMatrices();

                // Ahora iteramos sobre todas las meshes del modelo afectado y les pasamos las matrices de huesos
                for (auto& mesh : model->meshes) {
                    // Aqu� aplicar�as las matrices de huesos a los v�rtices de la mesh
                    ApplyBoneMatricesToMesh(mesh, finalBoneMatrices);
                }
            }
            else {
                std::cerr << "Error: El animador no est� inicializado." << std::endl;
            }
        }


        void ApplyBoneMatricesToMesh(Ref<Mesh> mesh, const std::vector<glm::mat4>& finalBoneMatrices) {
            for (Vertex& vertex : mesh->vertices) {
                glm::vec4 finalPosition(0.0f);

                // Aplicar la influencia de cada hueso en el v�rtice
                for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
                    if (vertex.m_BoneIDs[i] != -1) {  // Verifica que el hueso tenga influencia
                        int boneID = vertex.m_BoneIDs[i];
                        const glm::mat4& boneMatrix = finalBoneMatrices[boneID];

                        // Aplica la transformaci�n del hueso al v�rtice ponderado por el peso
                        finalPosition += boneMatrix * glm::vec4(vertex.position, 1.0f) * vertex.m_Weights[i];
                    }
                }

                // Actualiza la posici�n final del v�rtice en funci�n de la influencia de los huesos
                vertex.position = glm::vec3(finalPosition);
            }

            // Despu�s de actualizar los v�rtices, debes reenviar los datos al VBO
            mesh->SetupMesh();  // Esto volver� a cargar los datos de los v�rtices en la GPU
        }


        // Funci�n que devuelve las matrices finales de los huesos
        std::vector<glm::mat4> GetFinalBoneMatrices() {
            if (animator && animator->HasAnimation()) 
            {
                return animator->GetFinalBoneMatrices();
            }
            else {
                // Devuelve una matriz de identidad si no hay animaci�n activa
                return std::vector<glm::mat4>(100, glm::mat4(1.0f));  // Ajusta el tama�o seg�n tus necesidades
            }
        }
    };
}