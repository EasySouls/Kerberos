#include "kbrpch.h"
#include "EditorCamera.h"

#include "Kerberos/Core/Input.h"

namespace Kerberos
{
	EditorCamera::EditorCamera(const float fov, const float aspectRatio, const float nearClip, const float farClip)
		: Camera(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip)), m_Fov(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip)
	{
		UpdateView();
	}

	void EditorCamera::OnUpdate(Timestep deltaTime) 
	{
		if (Input::IsKeyPressed(Key::LeftAlt))
		{
			const glm::vec2 mouse = { Input::GetMouseX(), Input::GetMouseY() };
			const glm::vec2& delta = (mouse - m_InitialMousePosition) * 0.003f;
			m_InitialMousePosition = mouse;

			if (Input::IsMouseButtonPressed(Mouse::ButtonMiddle))
			{
				MousePan(delta);
			}
			else if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
			{
				MouseRotate(delta);
			}
			else if (Input::IsMouseButtonPressed(Mouse::ButtonLeft))
			{
				MouseZoom(delta.y);
			}

			UpdateView();
		}
	}

	void EditorCamera::OnEvent(Event& e) 
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(KBR_BIND_EVENT_FN(EditorCamera::OnMouseScroll));
	}

	void EditorCamera::SetViewportSize(const float width, const float height) 
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
		UpdateProjection();
	}

	glm::vec3 EditorCamera::GetUp() const 
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetRight() const 
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetForward() const 
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::quat EditorCamera::GetOrientation() const
	{
		return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}

	void EditorCamera::UpdateProjection() 
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		m_Projection = glm::perspective(glm::radians(m_Fov), m_AspectRatio, m_NearClip, m_FarClip);
	}

	void EditorCamera::UpdateView() 
	{

		// Lock the camera's rotation
		// m_Pitch = 0.0f;
		// m_Yaw = 0.0f;

		m_Position = CalculatePosition();

		const glm::quat orientation = GetOrientation();
		m_View = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_View = glm::inverse(m_View);
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e) 
	{
		const float delta = e.GetYOffset() * 0.1f;
		MouseZoom(delta);

		UpdateView();

		return false;
	}

	void EditorCamera::MousePan(const glm::vec2& delta) 
	{
		auto [xFactor, yFactor] = GetPanSpeed();
		m_FocalPoint += -GetRight() * delta.x * xFactor * m_Distance;
		m_FocalPoint += GetUp() * delta.y * yFactor * m_Distance;
	}

	void EditorCamera::MouseRotate(const glm::vec2& delta) 
	{
		const float yawSign = GetUp().y < 0.0f ? -1.0f : 1.0f;
		const float rotationSpeed = GetRotationSpeed();

		m_Yaw += delta.x * rotationSpeed * yawSign;
		m_Pitch += delta.y * rotationSpeed;
	}

	void EditorCamera::MouseZoom(const float delta) 
	{
		m_Distance -= delta * GetZoomSpeed();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += GetForward();
			m_Distance = 1.0f;
		}
	}

	glm::vec3 EditorCamera::CalculatePosition() const 
	{
		return m_FocalPoint - GetForward() * m_Distance;
	}

	std::pair<float, float> EditorCamera::GetPanSpeed() const 
	{
		const float x = std::min(m_ViewportWidth / 1000.f, 2.0f);
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		const float y = std::min(m_ViewportHeight / 1000.f, 2.0f);
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::GetRotationSpeed() const 
	{
		return 0.8f;
	}

	float EditorCamera::GetZoomSpeed() const 
	{
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);

		float speed = distance * distance;
		speed = std::min(speed, 100.0f);

		return speed;
	}
}
