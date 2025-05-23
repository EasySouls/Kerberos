#pragma once

#include "Kerberos/Core/Timestep.h"
#include "Kerberos/Events/Event.h"
#include "Kerberos/Renderer/Camera.h"
#include "Kerberos/Events/MouseEvent.h"

#include <glm/gtx/quaternion.hpp>

namespace Kerberos
{
	class EditorCamera final : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		void OnUpdate(Timestep deltaTime);
		void OnEvent(Event& e);

		float GetDistance() const { return m_Distance; }
		void SetDistance(const float distance) { m_Distance = distance; }

		void SetViewportSize(float width, float height);

		const glm::mat4& GetViewMatrix() const { return m_View; }
		glm::mat4 GetViewProjectionMatrix() const { return m_Projection * m_View; }

		glm::vec3 GetUp() const;
		glm::vec3 GetRight() const;
		glm::vec3 GetForward() const;
		const glm::vec3& GetPosition() const { return m_Position; }
		glm::quat GetOrientation() const { return m_Orientation; }

		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }

	private:
		void UpdateProjection();
		void UpdateView();

		bool OnMouseScroll(MouseScrolledEvent& e);

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		glm::vec3 CalculatePosition() const;

		std::pair<float, float> GetPanSpeed() const;
		float GetRotateSpeed() const;
		float GetZoomSpeed() const;

	private:
		float m_Fov = 45.0f;
		float m_AspectRatio = 1.778f;
		float m_NearClip = 0.1f;
		float m_FarClip = 1000.0f;

		glm::mat4 m_View;
		glm::vec3 m_Position = { 0.0f, 0.0f, -5.0f };
		glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

		glm::vec2 m_InitialMousePosition = { 0.0f, 0.0f };

		float m_Distance = 0.0f;
		float m_Pitch = 0.0f;
		float m_Yaw = 0.0f;
		glm::quat m_Orientation = glm::angleAxis(0.0f, glm::vec3(0.0f, 1.0f, 0.0f));

		float m_ViewportWidth = 0.0f;
		float m_ViewportHeight = 0.0f;
	};
}

