#pragma once
#include <Kerberos.h>

#include "ParticleSystem.h"

namespace Kerberos
{
	class EditorLayer : public Layer
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
		OrthographicCameraController m_CameraController;
		Ref<Framebuffer> m_Framebuffer;

		Ref<Scene> m_ActiveScene;

		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };

		glm::vec4 m_SquareColor = { 0.8f, 0.3f, 0.2f, 1.0f };

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

		ParticleSystem m_ParticleSystem;
		ParticleProps m_Particle;

		std::unordered_map<char, Ref<SubTexture2D>> m_TileMap;
		size_t m_MapWidth = 32;
		size_t m_MapHeight = 16;
	};
}

