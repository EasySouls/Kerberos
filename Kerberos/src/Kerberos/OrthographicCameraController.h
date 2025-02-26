#pragma once

#include "Core/Timestep.h"
#include "Events/ApplicationEvent.h"
#include "Events/Event.h"
#include "Events/MouseEvent.h"
#include "Renderer/OrthographicCamera.h"

namespace Kerberos
{
	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false);

		void OnUpdate(Timestep deltaTime);
		void OnEvent(Event& e);

		OrthographicCamera& GetCamera() { return m_Camera; }
		const OrthographicCamera& GetCamera() const { return m_Camera; }

	private:
		bool OnMouseScrolled(const MouseScrolledEvent& e);
		bool OnWindowResized(const WindowResizeEvent& e);

	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;

		bool m_RotationEnabled = false;

		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraRotation = 0.0f;

		float m_CameraMoveSpeed = 5.0f;
		float m_CameraRotationSpeed = 90.0f;

		OrthographicCamera m_Camera;
	};
}
