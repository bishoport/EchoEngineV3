#include "EntityManager.h"

#include "../core/Engine.h"

#include "../core/model/Model.h"
#include <minwindef.h>
#include <libloaderapi.h>
#include "LuaManager.h"


namespace libCore
{
    EntityManager& EntityManager::GetInstance() {
        static EntityManager instance;
        return instance;
    }
    EntityManager::EntityManager() {
        if (!m_registry) {
            m_registry = CreateScope<entt::registry>();
        }
    }

    //--CARGADOR DLL SCRIPTS
    std::wstring EntityManager::GetExecutablePath() {
        wchar_t buffer[MAX_PATH];
        GetModuleFileNameW(NULL, buffer, MAX_PATH);
        std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
        return std::wstring(buffer).substr(0, pos);
    }
    std::string EntityManager::wstring_to_string(const std::wstring& wstr) {
        std::string str(wstr.begin(), wstr.end());
        return str;
    }
    void EntityManager::LoadScriptsFromDLL() {
        std::wstring buildDir = GetExecutablePath();
        std::wstring dllPath = buildDir + L"\\TestScript.dll";

        LuaManager::GetInstance();
    }
    //------------------------------------------------------------------------------------

    //--ACCESOS
    entt::entity EntityManager::GetEntityByUUID(const std::string& uuid) {
        // Imprime el UUID que estás buscando
        //std::cout << "Buscando entidad con UUID: " << uuid << std::endl;

        auto view = m_registry->view<IDComponent>();
        for (auto entity : view) {
            auto& idComponent = view.get<IDComponent>(entity);
            std::string entityUUID = idComponent.ID.ToString();

            // Imprime cada UUID que se está revisando
            //std::cout << "Revisando entidad con UUID: " << entityUUID << " (Entity ID: " << static_cast<uint32_t>(entity) << ")" << std::endl;

            if (entityUUID == uuid) {
                //std::cout << "ENTITY_MANAGER -> Se encontró la entidad. Direccion de m_Entity: " << static_cast<uint32_t>(entity) << std::endl;
                return entity;
            }
        }

        // Si no se encontró la entidad, imprime un mensaje
        std::cerr << "ENTITY_MANAGER -> No se encontró ninguna entidad con UUID: " << uuid << std::endl;

        return entt::null; // Retorna null si no se encuentra la entidad
    }
    entt::registry* EntityManager::GetRegistry() {
        assert(m_registry.get() != nullptr && "m_registry debe estar inicializado");
        return m_registry.get();
    }
    //------------------------------------------------------------------------------------


    //--CREADOR DE ENTITIES
    entt::entity EntityManager::CreateEmptyGameObject(const std::string& name) {
        entt::entity entity = m_registry->create();
        m_registry->emplace<IDComponent>(entity, UUID());
        auto& tag = m_registry->emplace<TagComponent>(entity);
        tag.Tag = name.empty() ? "Entity" : name;
        m_registry->emplace<TransformComponent>(entity);


        if(Engine::GetInstance().GetEngineState() == PLAY)
        {
            m_registry->emplace<CreatedInRunTimeComponent>(entity);
        }

        return entity;
    }
    entt::entity EntityManager::CreateGameObjectFromModel(Ref<Model> model, entt::entity parent)
    {
        // Crear una nueva entidad para el modelo
        entt::entity entity = CreateEmptyGameObject(model->name);

        // Crear un nuevo TransformComponent para la entidad
        auto& transformComponent = GetComponent<TransformComponent>(entity);
        transformComponent.transform->position = model->transform->position;

        // Asignar los componentes de MaterialComponent si el modelo tiene materiales
        if (!model->materials.empty()) {
            for (auto& material : model->materials) {
                m_registry->emplace<MaterialComponent>(entity, material);
            }
        }

        // Asignar los componentes de MeshComponent si el modelo tiene mallas
        if (!model->meshes.empty()) {
            //MESHES
            for (auto& mesh : model->meshes) {
                auto& meshComponent = m_registry->emplace<MeshComponent>(entity, mesh, model, true);
                // Agregar matriz de instancia
                glm::mat4 instanceMatrix = glm::translate(glm::mat4(1.0f), model->transform->position);
                meshComponent.instanceMatrices.push_back(instanceMatrix);
                auto& abbComponent = m_registry->emplace<AABBComponent>(entity);
                abbComponent.aabb->CalculateAABB(mesh->vertices);
                meshComponent.originalModel = model;
            }
            //SKELETAL
            if (model->importModelData.skeletal == true)
            {
                auto& animationComponent = m_registry->emplace<AnimationComponent>(entity,model);
            }
        }

        // Asignar los componentes de herencia
        if (parent != entt::null) {
            AddChild(parent, entity);
        }

        // Recorrer y crear entidades para los hijos del modelo
        for (auto& child : model->children) {
            CreateGameObjectFromModel(child, entity);
        }

        return entity;
    }
    void EntityManager::CreateCamera()
    {
        entt::entity gameObject = CreateEmptyGameObject("MainCamera");
        AddComponent<CameraComponent>(gameObject);
    }
    entt::entity EntityManager::GetEntityByName(const std::string& name)
    {
        auto view = m_registry->view<TagComponent>();
        for (auto entity : view) {
            auto& tag = view.get<TagComponent>(entity);
            if (tag.Tag == name) {
                return entity;
            }
        }
        return entt::null; // Si no se encuentra ninguna entidad con el nombre dado
    }
    void EntityManager::AddChild(entt::entity parent, entt::entity child)
    {
        if (!m_registry->valid(parent) || !m_registry->valid(child)) {
            return;
        }

        // Obtener la transformación global actual del hijo
        auto& childTransformComponent = GetComponent<TransformComponent>(child);
        glm::mat4 childGlobalTransform = childTransformComponent.getGlobalTransform(child, *m_registry);

        // Eliminar el hijo de la lista de hijos del antiguo padre, si lo tiene
        if (HasComponent<ParentComponent>(child)) {
            entt::entity oldParent = GetComponent<ParentComponent>(child).parent;
            if (m_registry->valid(oldParent) && HasComponent<ChildComponent>(oldParent)) {
                auto& oldParentChildren = GetComponent<ChildComponent>(oldParent).children;
                oldParentChildren.erase(std::remove(oldParentChildren.begin(), oldParentChildren.end(), child), oldParentChildren.end());
            }
        }

        // Añadir ParentComponent al hijo
        if (!HasComponent<ParentComponent>(child)) {
            AddComponent<ParentComponent>(child).parent = parent;
        }
        else {
            GetComponent<ParentComponent>(child).parent = parent;
        }

        // Añadir ChildComponent al padre
        if (!HasComponent<ChildComponent>(parent)) {
            AddComponent<ChildComponent>(parent).children.push_back(child);
        }
        else {
            auto& parentChildren = GetComponent<ChildComponent>(parent).children;
            if (std::find(parentChildren.begin(), parentChildren.end(), child) == parentChildren.end()) {
                parentChildren.push_back(child);
            }
        }

        // Actualizar la transformación local del hijo para mantener la transformación global
        childTransformComponent.setTransformFromGlobal(childGlobalTransform, child, *m_registry);
    }
    //------------------------------------------------------------------------------------

    //--DUPLICATE
    void EntityManager::DuplicateEntity()
    {
        if (EntityManager::GetInstance().currentSelectedEntityInScene != entt::null) {
            entt::entity selectedEntity = EntityManager::GetInstance().currentSelectedEntityInScene;
            entt::entity parentEntity = entt::null;

            // Verificar si la entidad seleccionada tiene un componente de padre
            if (EntityManager::GetInstance().HasComponent<ParentComponent>(selectedEntity)) {
                parentEntity = EntityManager::GetInstance().GetComponent<ParentComponent>(selectedEntity).parent;
            }
            entt::entity newEntity = DuplicateEntityRecursively(selectedEntity, parentEntity);
            UpdateAccumulatedTransforms(newEntity);
        }
    }
    entt::entity EntityManager::DuplicateEntityRecursively(entt::entity entity, entt::entity parentEntity)
    {
        if (!m_registry->valid(entity)) {
            return entt::null;
        }

        // Obtener el nombre original
        std::string originalTag = GetComponent<TagComponent>(entity).Tag;
        std::string newTag = originalTag;

        // Buscar si ya existen clones y agregar un sufijo único
        int cloneIndex = 1;
        while (IsTagUsed(newTag + "_" + std::to_string(cloneIndex))) {
            cloneIndex++;
        }
        newTag = originalTag + "_" + std::to_string(cloneIndex);

        // Crear una nueva entidad con el nombre modificado
        entt::entity newEntity = CreateEmptyGameObject(newTag);

        // Añadir el hijo al padre usando AddChild
        if (parentEntity != entt::null) {
            AddChild(parentEntity, newEntity);
        }

        // Manejar los hijos
        if (HasComponent<ChildComponent>(entity)) {
            auto& srcChild = GetComponent<ChildComponent>(entity);
            for (auto& child : srcChild.children) {
                DuplicateEntityRecursively(child, newEntity);
            }
        }



        // Copiar componentes
        if (HasComponent<TransformComponent>(entity)) {
            auto& srcTransform = GetComponent<TransformComponent>(entity);
            auto& dstTransform = GetComponent<TransformComponent>(newEntity);

            dstTransform.transform->position =    srcTransform.transform->position;
            dstTransform.transform->scale =       srcTransform.transform->scale;
            dstTransform.transform->rotation =    srcTransform.transform->rotation;
            dstTransform.transform->eulerAngles = srcTransform.transform->eulerAngles;
        }

        if (HasComponent<MeshComponent>(entity)) {
            auto& srcMesh = GetComponent<MeshComponent>(entity);
            auto& dstMesh = AddComponent<MeshComponent>(newEntity, srcMesh.mesh, srcMesh.originalModel, srcMesh.isInstance);

            // Agregar la matriz de instancia al nuevo MeshComponent
            glm::mat4 instanceMatrix = glm::translate(glm::mat4(1.0f), GetComponent<TransformComponent>(newEntity).transform->position);
            dstMesh.instanceMatrices.push_back(instanceMatrix);
        }

        if (HasComponent<MaterialComponent>(entity)) {
            auto& srcMaterial = GetComponent<MaterialComponent>(entity);
            AddComponent<MaterialComponent>(newEntity, srcMaterial.material);
        }

        if (HasComponent<AABBComponent>(entity)) {
            auto& srcAABB = GetComponent<AABBComponent>(entity);
            auto& dstAABB = AddComponent<AABBComponent>(newEntity);
            auto& srcMesh = GetComponent<MeshComponent>(entity);
            dstAABB.aabb->CalculateAABB(srcMesh.mesh->vertices);
        }

        //--todo
        //if (HasComponent<ScriptComponent>(entity)) {
        //    auto& srcScript = GetComponent<ScriptComponent>(entity);
        //    ImportLUA_ScriptData scriptData = srcScript.GetLuaScriptData();  // Obtener el script y sus datos

        //    // Asignar el script al nuevo entity
        //    auto& dstScript = AddComponent<ScriptComponent>(newEntity);
        //    dstScript.SetLuaScript(scriptData);

        //    // Copiar las exposedVars
        //    auto srcExposedVars = srcScript.GetExposedVars();
        //    dstScript.SetExposedVars(srcExposedVars);
        //}

        return newEntity;
    }
    bool EntityManager::IsTagUsed(const std::string& tag)
    {
        auto view = m_registry->view<TagComponent>();
        for (auto entity : view) {
            auto& tagComponent = view.get<TagComponent>(entity);
            if (tagComponent.Tag == tag) {
                return true;
            }
        }
        return false;
    }
    void EntityManager::UpdateAccumulatedTransforms(entt::entity entity, const glm::mat4& parentTransform)
    {
        if (!m_registry->valid(entity)) {
            return;
        }

        auto& transformComponent = GetComponent<TransformComponent>(entity);

        // Actualizar la transformación acumulada de la entidad actual
        transformComponent.accumulatedTransform = parentTransform * transformComponent.transform->getLocalModelMatrix();

        // Recorrer recursivamente los hijos
        if (HasComponent<ChildComponent>(entity)) {
            auto& childComponent = GetComponent<ChildComponent>(entity);
            for (auto& child : childComponent.children) {
                UpdateAccumulatedTransforms(child, transformComponent.accumulatedTransform);
            }
        }
    }
    //------------------------------------------------------------------------------------

    //--DESTROY
    void EntityManager::MarkToDeleteRecursively(entt::entity entity)
    {
        if (!m_registry->valid(entity)) {
            return;
        }

        // Marcar la entidad actual
        if (HasComponent<IDComponent>(entity)) {
            GetComponent<IDComponent>(entity).markToDelete = true;
        }

        // Marcar recursivamente los hijos
        if (HasComponent<ChildComponent>(entity)) {
            auto& childComponent = GetComponent<ChildComponent>(entity);
            for (auto& child : childComponent.children) {
                MarkToDeleteRecursively(child);
            }
        }
    }
    void EntityManager::DestroyEntity(entt::entity entity)
    {
        if (m_registry->valid(entity)) {
            m_registry->destroy(entity);
        }
    }
    void EntityManager::DestroyEntityRecursively(entt::entity entity)
    {
        if (!m_registry->valid(entity)) {
            return;
        }

        // Primero, eliminar todos los hijos recursivamente
        if (HasComponent<ChildComponent>(entity)) {
            auto& childComponent = GetComponent<ChildComponent>(entity);
            for (auto childEntity : childComponent.children) {
                DestroyEntityRecursively(childEntity);
            }
        }

        // Eliminar referencia del padre si existe
        if (HasComponent<ParentComponent>(entity)) {
            auto& parentComponent = GetComponent<ParentComponent>(entity);
            if (m_registry->valid(parentComponent.parent)) {
                auto& parentChildren = GetComponent<ChildComponent>(parentComponent.parent).children;
                parentChildren.erase(std::remove(parentChildren.begin(), parentChildren.end(), entity), parentChildren.end());
            }
        }

        if (HasComponent<ScriptComponent>(entity)) {
            // Si tienes alguna limpieza especial para el script, hazla aquí
            RemoveComponent<ScriptComponent>(entity);  // Elimina el ScriptComponent
        }


        // Luego, eliminar la entidad actual
        DestroyEntity(entity);
    }
    void EntityManager::DestroyAllEntities()
    {
        // Primero, marcar todas las entidades para ser eliminadas
        m_registry->each([this](auto entity) {
            MarkToDeleteRecursively(entity);  // Marca todas las entidades para ser eliminadas
            });

        // Ahora eliminar recursivamente todas las entidades
        m_registry->each([this](auto entity) {
            DestroyEntityRecursively(entity);  // Elimina todas las entidades y sus hijos
            });

        // Después de eliminar las entidades, limpia el registro de cualquier entidad residual
        m_registry->clear();
    }
    //------------------------------------------------------------------------------------


    //--INIT SCRIPTS Components
    void EntityManager::InitScripts() 
    {
        auto scriptView = m_registry->view<ScriptComponent>();
        
        for (auto entity : scriptView) {
            auto& scriptComponent = scriptView.get<ScriptComponent>(entity);
            scriptComponent.Init();
        }
    }
    void EntityManager::UpdateScripts(Timestep deltaTime) 
    {
        auto scriptView = m_registry->view<ScriptComponent>();
        for (auto entity : scriptView) {
            auto& scriptComponent = scriptView.get<ScriptComponent>(entity);
            scriptComponent.Update(deltaTime);
        }
    }
    //------------------------------------------------------------------------------------



    //--ACTUALIZADOR DE FUNCIONES UPDATES ANTES DEL RENDER DE LOS COMPONENTES
    void EntityManager::UpdateGameObjects(Timestep deltaTime)
    {
        //Destruccion real antes de actualizar
        //(Quizá mejor crear un componente temporal para esto)
        auto IDCompView = m_registry->view<IDComponent>();
        for (auto entity : IDCompView) {
            auto& idComponent = GetComponent<IDComponent>(entity);

            if (idComponent.markToDelete == true) {
                EntityManager::GetInstance().DestroyEntityRecursively(entity);
            }
        }

        // ACTUALIZACION DE LOS ANIMATION
        auto viewAnimation = m_registry->view<AnimationComponent>();
        for (auto entity : viewAnimation) {
            auto& animationComponent = viewAnimation.get<AnimationComponent>(entity);

            // Actualiza el tiempo y el estado de la animación
            animationComponent.Update(deltaTime);
        }

        // Actualizar scripts (si es necesario)
        if (runScripting) {
            UpdateScripts(deltaTime);
        }

        //UPDATE ALL TRANSFORM CHILDREN
        auto rootView = m_registry->view<TransformComponent>(entt::exclude<ParentComponent>);
        for (auto entity : rootView) {
            UpdateAccumulatedTransforms(entity);
        }


        // Actualizar las cámaras
        //auto CameraCompView = m_registry->view<CameraComponent, TransformComponent>();
        //
        //for (auto entity : CameraCompView) {
        //    auto& cameraComponent = GetComponent<CameraComponent>(entity);
        //    auto& transformComponent = GetComponent<TransformComponent>(entity);
        //
        //    glm::vec3 forward = transformComponent.transform->rotation * glm::vec3(0.0f, 0.0f, -1.0f);
        //    cameraComponent.view = glm::lookAt(transformComponent.transform->position, transformComponent.transform->position + forward, cameraComponent.Up);
        //
        //    if (cameraComponent.width > 0 && cameraComponent.height > 0) {
        //        cameraComponent.projection = glm::perspective(glm::radians(cameraComponent.FOVdeg), (float)cameraComponent.width / (float)cameraComponent.height, cameraComponent.nearPlane, cameraComponent.farPlane);
        //    }
        //
        //    cameraComponent.cameraMatrix = cameraComponent.projection * cameraComponent.view;
        //}


        // Actualizar los AABB
        auto viewAABB = m_registry->view<TransformComponent, AABBComponent>();
        for (auto entity : viewAABB) {
            auto& transformComponent = viewAABB.get<TransformComponent>(entity);
            auto& aabbComponent = viewAABB.get<AABBComponent>(entity);

            glm::mat4 globalTransform = transformComponent.accumulatedTransform;
            aabbComponent.aabb->UpdateAABB(globalTransform);
        }
    }
    //------------------------------------------------------------------------------------

    //--DRAW MESH Component (Son llamadas desde el Renderer cuando le toque)
    void EntityManager::DrawGameObjects(const std::string& shader)
    {
        auto view = m_registry->view<TransformComponent, MeshComponent, MaterialComponent, AABBComponent>();
        for (auto entity : view) {
            DrawOneGameObject(entity, shader);
        }
    }
    void EntityManager::DrawOneGameObject(entt::entity entity, const std::string& shader)
    {
        auto& transformComponent = GetComponent<TransformComponent>(entity);
        auto& meshComponent = GetComponent<MeshComponent>(entity);
        auto& materialComponent = GetComponent<MaterialComponent>(entity);

        // Valores del material
        libCore::ShaderManager::Get(shader)->setVec3("albedoColor", materialComponent.material->albedoColor);
        libCore::ShaderManager::Get(shader)->setFloat("normalStrength", materialComponent.material->normalStrength);
        libCore::ShaderManager::Get(shader)->setFloat("metallicValue", materialComponent.material->metallicValue);
        libCore::ShaderManager::Get(shader)->setFloat("roughnessValue", materialComponent.material->roughnessValue);

        // Texturas
        materialComponent.material->albedoMap->Bind(shader);
        materialComponent.material->normalMap->Bind(shader);
        materialComponent.material->metallicMap->Bind(shader);
        materialComponent.material->roughnessMap->Bind(shader);

        // Usar la transformación acumulada
        libCore::ShaderManager::Get(shader)->setMat4("model", transformComponent.accumulatedTransform);

        //-SKELETAL
        bool useBones = false;

        if (HasComponent<AnimationComponent>(entity)) {
            auto& animationComponent = GetComponent<AnimationComponent>(entity);
            useBones = true;
            auto boneTransforms = animationComponent.GetFinalBoneMatrices();

            for (int i = 0; i < boneTransforms.size(); ++i) {
                libCore::ShaderManager::Get(shader)->setMat4("finalBonesMatrices[" + std::to_string(i) + "]", boneTransforms[i]);
                libCore::ShaderManager::Get(shader)->setFloat("boneScaleFactor", animationComponent.boneScaleFactor);
            }
        }
        // Establecer el valor de 'useBones' en el shader
        libCore::ShaderManager::Get(shader)->setBool("useBones", useBones);
        //---

        //DRAW INSTANCE
        if (!meshComponent.instanceMatrices.empty())
        {
            meshComponent.mesh->DrawInstanced(static_cast<GLsizei>(meshComponent.instanceMatrices.size()), meshComponent.instanceMatrices);
            //meshComponent.mesh->Draw();  //<-Dibujado sin Instancia (el modelo Original)
        }
    }
    //------------------------------------------------------------------------------------


    //--DRAW AABB Component (Son llamadas desde el Renderer cuando le toque)
    void EntityManager::DrawABBGameObjectMeshComponent(const std::string& shader)
    {
        auto viewAABB = m_registry->view<TransformComponent, AABBComponent>();
        for (auto entity : viewAABB) {
            auto& transformComponent = viewAABB.get<TransformComponent>(entity);
            auto& aabbComponent = viewAABB.get<AABBComponent>(entity);

            if (aabbComponent.aabb->showAABB) {
                // Obtener la transformación global correcta
                glm::mat4 globalTransform = transformComponent.getGlobalTransform(entity, *m_registry);

                libCore::ShaderManager::Get(shader)->setVec4("u_Color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                libCore::ShaderManager::Get(shader)->setMat4("model", globalTransform);

                aabbComponent.aabb->DrawAABB();
            }
        }
    }
    //------------------------------------------------------------------------------------

    //--AABB Component MOUSE CHECKER
    void EntityManager::CheckRayModelIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDirection)
    {
        auto viewAABB = m_registry->view<TransformComponent, AABBComponent>();
        for (auto entity : viewAABB) {
            auto& transformComponent = viewAABB.get<TransformComponent>(entity);
            auto& aabbComponent = viewAABB.get<AABBComponent>(entity);
            // Obtener la transformación global correcta
            glm::mat4 globalTransform = transformComponent.getGlobalTransform(entity, *m_registry);
            // Transformar AABB
            glm::vec3 transformedMin = glm::vec3(globalTransform * glm::vec4(aabbComponent.aabb->minBounds, 1.0));
            glm::vec3 transformedMax = glm::vec3(globalTransform * glm::vec4(aabbComponent.aabb->maxBounds, 1.0));
            // Verificar la intersección del rayo con la AABB transformada
            if (rayIntersectsBoundingBox(rayOrigin, rayDirection, transformedMin, transformedMax)) {
                entitiesInRay.push_back(entity);
            }
        }
    }
    bool EntityManager::rayIntersectsBoundingBox(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, glm::vec3 boxMin, glm::vec3 boxMax)
    {
        float tMin = (boxMin.x - rayOrigin.x) / rayDirection.x;
        float tMax = (boxMax.x - rayOrigin.x) / rayDirection.x;
        if (tMin > tMax) std::swap(tMin, tMax);
        float tyMin = (boxMin.y - rayOrigin.y) / rayDirection.y;
        float tyMax = (boxMax.y - rayOrigin.y) / rayDirection.y;
        if (tyMin > tyMax) std::swap(tyMin, tyMax);
        if ((tMin > tyMax) || (tyMin > tMax)) return false;
        if (tyMin > tMin) tMin = tyMin;
        if (tyMax < tMax) tMax = tyMax;
        float tzMin = (boxMin.z - rayOrigin.z) / rayDirection.z;
        float tzMax = (boxMax.z - rayOrigin.z) / rayDirection.z;
        if (tzMin > tzMax) std::swap(tzMin, tzMax);
        if ((tMin > tzMax) || (tzMin > tMax)) return false;
        if (tzMin > tMin) tMin = tzMin;
        if (tzMax < tMax) tMax = tzMax;
        return true;
    }
    //------------------------------------------------------------------------------------


    //--ESPECIALES
    void libCore::EntityManager::CheckInstancesInRunTime()
    {
        auto viewCreatedInRunTimeComponent = m_registry->view<CreatedInRunTimeComponent>();
        for (auto entity : viewCreatedInRunTimeComponent) {
            if (HasComponent<IDComponent>(entity)) {
                GetComponent<IDComponent>(entity).markToDelete = true;
            }
        }
    }


    //DEBUG ENTITIES
    void EntityManager::PrintEntityInfo(entt::entity entity, const Ref<entt::registry>& registry)
    {
        std::cout << "Entity ID: " << static_cast<uint32_t>(entity);

        if (registry->has<TagComponent>(entity)) {
            auto& tag = registry->get<TagComponent>(entity);
            std::cout << " (Tag: " << tag.Tag << ")";
        }

        std::cout << std::endl;

        if (registry->has<ParentComponent>(entity)) {
            auto& parent = registry->get<ParentComponent>(entity);
            std::cout << "  Parent ID: " << static_cast<uint32_t>(parent.parent) << std::endl;
        }

        if (registry->has<ChildComponent>(entity)) {
            auto& children = registry->get<ChildComponent>(entity).children;
            std::cout << "  Children: ";
            for (auto& child : children) {
                std::cout << static_cast<uint32_t>(child) << " ";
            }
            std::cout << std::endl;
        }
    }
    void EntityManager::DebugPrintAllEntities()
    {
        /*auto& registry = EntityManager::GetInstance().m_registry;

        registry->each([&](entt::entity entity) {
            PrintEntityInfo(entity, registry);
            });*/
    }
    void EntityManager::DebugPrintEntityHierarchy(entt::entity entity, const Ref<entt::registry>& registry, int level)
    {
        for (int i = 0; i < level; ++i) {
            std::cout << "  ";
        }

        PrintEntityInfo(entity, registry);

        if (registry->has<ChildComponent>(entity)) {
            auto& children = registry->get<ChildComponent>(entity).children;
            for (auto& child : children) {
                DebugPrintEntityHierarchy(child, registry, level + 1);
            }
        }
    }
    void EntityManager::DebugPrintAllEntityHierarchies()
    {
        //auto& registry = EntityManager::GetInstance().m_registry;

        //// Filtrar entidades que no tienen un padre (raíz)
        //auto rootView = registry->view<TransformComponent>(entt::exclude<ParentComponent>);
        //for (auto entity : rootView) {
        //    DebugPrintEntityHierarchy(entity, registry);
        //}
    }
    void EntityManager::DebugPrintAllEntitiesWithUUIDs()
    {
 
        auto view = GetRegistry()->view<IDComponent>();
        for (auto entity : view) {
            auto& idComponent = view.get<IDComponent>(entity);
            std::cout << "Entity: " << static_cast<uint32_t>(entity) << " UUID: " << idComponent.ID.ToString() << std::endl;
        }
    }
    //------------------------------------------------------------------------------------
}