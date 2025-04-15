#pragma once

#include <glm/glm.hpp>

namespace Kerberos
{
	class Camera
	{
	public:
		Camera()
			: m_Projection({ 1.0f })
		{}

		explicit Camera(const glm::mat4& projection)
			: m_Projection(projection)
		{}

		virtual ~Camera() = default;

		const glm::mat4& GetProjection() const { return m_Projection; }

	protected:
		glm::mat4 m_Projection;
	};
}