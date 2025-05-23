#include "kbrpch.h"
#include "EditorCamera.h"

namespace Kerberos
{
	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip) {}
	void EditorCamera::OnUpdate(Timestep deltaTime) {}
	void EditorCamera::OnEvent(Event& e) {}

	void EditorCamera::SetViewportSize(const float width, const float height) 
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
		UpdateProjection();
	}

	glm::vec3 EditorCamera::GetUp() const {}
	glm::vec3 EditorCamera::GetRight() const {}
	glm::vec3 EditorCamera::GetForward() const {}
	void EditorCamera::UpdateProjection() {}
	void EditorCamera::UpdateView() {}
	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e) {}
	void EditorCamera::MousePan(const glm::vec2& delta) {}
	void EditorCamera::MouseRotate(const glm::vec2& delta) {}
	void EditorCamera::MouseZoom(float delta) {}
	glm::vec3 EditorCamera::CalculatePosition() const {}
	std::pair<float, float> EditorCamera::GetPanSpeed() const {}
	float EditorCamera::GetRotateSpeed() const {}
	float EditorCamera::GetZoomSpeed() const {}
}
