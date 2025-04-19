#pragma once

#include "Kerberos/Core.h"
#include "Kerberos/Scene/Scene.h"
#include "Kerberos/Scene/Entity.h"

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

	private:
		void DrawEntityNode(const Entity& entity);

	private:
		Ref<Scene> m_Context;

		Entity m_SelectedEntity;
	};
}