#include "kbrpch.h"

#include "OrthographicCameraController.h"

#include "Kerberos/Core/Input.h"
#include "Kerberos/Core/KeyCodes.h"


namespace Kerberos
{
	OrthographicCameraController::OrthographicCameraController(const float aspectRatio, const bool rotation)
		: m_AspectRatio(aspectRatio), m_RotationEnabled(rotation), m_Bounds({ .Left = -m_AspectRatio * m_ZoomLevel, .Right = m_AspectRatio * m_ZoomLevel, .Bottom = -m_ZoomLevel, .Top = m_ZoomLevel }),
		  m_Camera(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top)
	{
	}

	void OrthographicCameraController::OnUpdate(const Timestep deltaTime)
	{
		/// Move the camera in the x axis
		if (Input::IsKeyPressed(Key::A))
			m_CameraPosition.x -= m_CameraMoveSpeed * deltaTime;
		else if (Input::IsKeyPressed(Key::D))
			m_CameraPosition.x += m_CameraMoveSpeed * deltaTime;

		/// Move the camera in the y axis
		if (Input::IsKeyPressed(Key::W))
			m_CameraPosition.y += m_CameraMoveSpeed * deltaTime;
		else if (Input::IsKeyPressed(Key::S))
			m_CameraPosition.y -= m_CameraMoveSpeed * deltaTime;

		m_Camera.SetPosition(m_CameraPosition);

		/// Rotate the camera if rotation is enabled
		if (m_RotationEnabled)
		{
			if (Input::IsKeyPressed(Key::Q))
				m_CameraRotation += m_CameraRotationSpeed * deltaTime;
			else if (Input::IsKeyPressed(Key::E))
				m_CameraRotation -= m_CameraRotationSpeed * deltaTime;

			m_Camera.SetRotation(m_CameraRotation);
		}
	}

	void OrthographicCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(KBR_BIND_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(KBR_BIND_FN(OrthographicCameraController::OnWindowResized));
	}

	void OrthographicCameraController::OnResize(const float width, const float height) 
	{
		m_AspectRatio = width / height;

		CalculateView();
	}

	bool OrthographicCameraController::OnMouseScrolled(const MouseScrolledEvent& e)
	{
		m_ZoomLevel -= e.GetYOffset() * 0.25f;

		// / Clamp the zoom level
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);

		CalculateView();

		return false;
	}

	bool OrthographicCameraController::OnWindowResized(const WindowResizeEvent& e)
	{
		m_AspectRatio = static_cast<float>(e.GetWidth()) / static_cast<float>(e.GetHeight());

		CalculateView();

		return false;
	}

	void OrthographicCameraController::CalculateView() 
	{
		m_Bounds = { .Left = -m_AspectRatio * m_ZoomLevel, .Right = m_AspectRatio * m_ZoomLevel, .Bottom = -m_ZoomLevel, .Top = m_ZoomLevel };
		m_Camera.SetProjection(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top);
	}
}
