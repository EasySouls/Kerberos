#include "kbrpch.h"
#include "OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Kerberos
{
	OrthographicCamera::OrthographicCamera(const float left, const float right, const float bottom, const float top)
		: m_ProjectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)), m_ViewMatrix(1.0f)
	{
		KBR_PROFILE_FUNCTION();

		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::SetProjection(const float left, const float right, const float bottom, const float top)
	{
		KBR_PROFILE_FUNCTION();

		m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::RecalculateViewMatrix() 
	{
		KBR_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));

		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::SetPosition(const glm::vec3& position) 
	{
		KBR_PROFILE_FUNCTION();

		m_Position = position;
		RecalculateViewMatrix();
	}

	void OrthographicCamera::SetRotation(const float rotation) 
	{
		KBR_PROFILE_FUNCTION();

		m_Rotation = rotation;
		RecalculateViewMatrix();
	}
}
