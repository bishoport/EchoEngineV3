#pragma once
#include "PanelBase.h"
#include "../../managers/EntityManager.h"

namespace libCore
{
    class HierarchyPanel : public PanelBase
    {
    public:
        HierarchyPanel() : PanelBase("Hierarchy") {}

        void Init() override
        {

        }


        void DrawEntityNode(entt::entity entity) {
            ImGui::PushID(static_cast<int>(entity));

            auto& tagComponent = EntityManager::GetInstance().m_registry->get<TagComponent>(entity);
            auto& idComponent = EntityManager::GetInstance().m_registry->get<IDComponent>(entity);

            std::string uuidStr = std::to_string(static_cast<uint64_t>(idComponent.ID));
            bool isParent = EntityManager::GetInstance().m_registry->has<ChildComponent>(entity);
            bool isChild = EntityManager::GetInstance().m_registry->has<ParentComponent>(entity);

            std::string nodeName = tagComponent.Tag;

            bool isSelected = EntityManager::GetInstance().currentSelectedEntityInScene == entity;
            ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (isSelected ? ImGuiTreeNodeFlags_Selected : 0);
            if (!isParent) {
                nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            }

            // Mostrar un icono al lado de la entidad
            std::string icon = isParent ? ICON_FA_FOLDER_OPEN : ICON_FA_CUBE; // Icono para entidad padre o hija
            bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)entity, nodeFlags, "%s %s", icon.c_str(), nodeName.c_str());

            // Selección de entidad
            if (ImGui::IsItemClicked()) {
                EntityManager::GetInstance().currentSelectedEntityInScene = entity;
            }

            // Drag source
            if (ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("ENTITY_PAYLOAD", &entity, sizeof(entt::entity));
                ImGui::Text("%s %s", ICON_FA_ARROWS_ALT, nodeName.c_str());
                ImGui::EndDragDropSource();
            }

            // Drag target
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_PAYLOAD")) {
                    IM_ASSERT(payload->DataSize == sizeof(entt::entity));
                    entt::entity droppedEntity = *(const entt::entity*)payload->Data;
                    EntityManager::GetInstance().AddChild(entity, droppedEntity);
                }
                ImGui::EndDragDropTarget();
            }

            if (nodeOpen && isParent) {
                auto& children = EntityManager::GetInstance().m_registry->get<ChildComponent>(entity).children;
                for (auto child : children) {
                    DrawEntityNode(child);
                }
                ImGui::TreePop();
            }

            ImGui::PopID();
        }

        void Draw() override {
            if (!m_isVisible) return;

            ImGui::Begin(m_title.c_str());
            //-- Iconos para el título de la jerarquía

            // Iterar sobre las entidades raíz (aquellas que no tienen un ParentComponent)
            auto view = EntityManager::GetInstance().m_registry->view<TransformComponent>(entt::exclude<ParentComponent>);

            for (auto entity : view) {
                DrawEntityNode(entity);
            }

            ImGui::End();
        }

        void Shutdown() override {
            // Liberación de recursos si es necesario
        }
    };
}