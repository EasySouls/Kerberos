#pragma once

#include "Kerberos/Core.h"
#include "Kerberos/Scene/Scene.h"
#include "Kerberos/Scene/Entity.h"
#include "Kerberos/Renderer/Texture.h"
#include "Kerberos/Renderer/Material.h"
#include "Kerberos/Renderer/Mesh.h"
#include "../Notification/NotificationManager.h"

namespace Kerberos
{
	class HierarchyPanel
	{
	public:
		HierarchyPanel() = default;
		explicit HierarchyPanel(const Ref<Scene>& context);
		~HierarchyPanel() = default;

		void SetContext(const Ref<Scene>& context);

		void OnImGuiRender();
		void DrawComponents(Entity entity);

		Entity GetSelectedEntity() const { return m_SelectedEntity; }
		void SetSelectedEntity(Entity entity);

	private:
		void DrawEntityNode(const Entity& entity);

	private:
		Ref<Scene> m_Context;

		Entity m_SelectedEntity;

		// Examples
		Ref<Texture2D> m_IceTexture;
		Ref<Texture2D> m_SpriteSheetTexture;
		Ref<Texture2D> m_WhiteTexture;

		Ref<Mesh> m_CubeMesh;
		Ref<Mesh> m_SphereMesh;
		
		Ref<Material> m_WhiteMaterial;

		NotificationManager m_NotificationManager;
	};
}