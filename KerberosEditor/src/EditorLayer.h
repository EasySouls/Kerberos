#pragma once
#include <Kerberos.h>

//#include "ParticleSystem.h"
#include "Editor/HierarchyPanel.h"
#include "Editor/AssetsPanel.h"


namespace Kerberos
{
	class EditorLayer final : public Layer
	{
	public:
		EditorLayer();
		~EditorLayer() override = default;

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(Timestep deltaTime) override;
		void OnImGuiRender() override;
		void OnEvent(Event& event) override;

	private:
		bool OnKeyPressed(const KeyPressedEvent& event);
		bool OnMouseButtonPressed(const MouseButtonPressedEvent& event);

		void OnScenePlay();
		void OnSceneStop();

		void HandleDragAndDrop();

		void CalculateEntityTransforms() const;

		void SaveScene() const;
		void SaveSceneAs() const;
		void LoadScene();
		void NewScene();

		void UIToolbar();

	private:
		OrthographicCameraController m_CameraController;
		Ref<Framebuffer> m_Framebuffer;

		EditorCamera m_EditorCamera;

		Ref<Scene> m_ActiveScene;
		Entity m_CameraEntity;
		Entity m_SecondCamera;
		Entity m_SunlightEntity;

		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		std::array<glm::vec2, 2> m_ViewportBounds;

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;

		Entity m_HoveredEntity;

		glm::vec4 m_SquareColor = { 0.8f, 0.3f, 0.2f, 1.0f };

		/// Used when toggling the cameras
		bool m_IsPrimaryCamera = true;

		bool m_IsScene3D = true;

		int m_GizmoType = -1;

		/// Later this should be handled by the ECS system
		bool m_RenderSkybox = false;
		bool m_RenderOceanSkybox = true;

		bool m_ShowWireframe = false;

		Ref<Texture2D> m_Texture;
		Ref<Texture2D> m_SpriteSheet;
		Ref<SubTexture2D> m_TextureStairs, m_TextureBarrel, m_TextureTree, m_TextureGrass, m_TextureDirt, m_TextureWater;

		struct ProfileResult
		{
			const char* Name;
			float Time;
		};

		float m_Fps = 0;

		std::vector<ProfileResult> m_ProfileResults;

		/// Editor Panels
		HierarchyPanel m_HierarchyPanel;
		AssetsPanel m_AssetsPanel;

		enum class SceneState : uint8_t
		{
			Edit,
			Play,
			Simulate
		};

		Ref<Texture2D> m_IconPlay;
		Ref<Texture2D> m_IconStop;
		SceneState m_SceneState = SceneState::Edit;
	};
}

