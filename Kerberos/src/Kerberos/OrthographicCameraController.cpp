#include "kbrpch.h"

#include "OrthographicCameraController.h"

#include "Kerberos/Input.h"
#include "Kerberos/KeyCodes.h"


namespace Kerberos
{
	OrthographicCameraController::OrthographicCameraController(const float aspectRatio, const bool rotation)
		: m_AspectRatio(aspectRatio), m_RotationEnabled(rotation), m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio* m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel)
	{
	}

	void OrthographicCameraController::OnUpdate(const Timestep deltaTime)
	{
		/// Move the camera in the x axis
		if (Input::IsKeyPressed(KBR_KEY_A))
			m_CameraPosition.x -= m_CameraMoveSpeed * deltaTime;
		else if (Input::IsKeyPressed(KBR_KEY_D))
			m_CameraPosition.x += m_CameraMoveSpeed * deltaTime;

		/// Move the camera in the y axis
		if (Input::IsKeyPressed(KBR_KEY_W))
			m_CameraPosition.y += m_CameraMoveSpeed * deltaTime;
		else if (Input::IsKeyPressed(KBR_KEY_S))
			m_CameraPosition.y -= m_CameraMoveSpeed * deltaTime;

		m_Camera.SetPosition(m_CameraPosition);

		/// Rotate the camera if rotation is enabled
		if (m_RotationEnabled)
		{
			if (Input::IsKeyPressed(KBR_KEY_Q))
				m_CameraRotation += m_CameraRotationSpeed * deltaTime;
			else if (Input::IsKeyPressed(KBR_KEY_E))
				m_CameraRotation -= m_CameraRotationSpeed * deltaTime;

			m_Camera.SetRotation(m_CameraRotation);
		}
	}

	void OrthographicCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(KBR_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(KBR_BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));
	}

	bool OrthographicCameraController::OnMouseScrolled(const MouseScrolledEvent& e)
	{
		m_ZoomLevel -= e.GetYOffset() * 0.25f;

		// / Clamp the zoom level
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);

		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);

		return false;
	}

	bool OrthographicCameraController::OnWindowResized(const WindowResizeEvent& e)
	{
		m_AspectRatio = static_cast<float>(e.GetWidth()) / static_cast<float>(e.GetHeight());
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);

		return false;
	}
}
