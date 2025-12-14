#pragma once

#include <glm/glm.hpp>

namespace Kerberos
{
	struct ComputeInfo
	{
		glm::vec3 MaxWorkGroupCount{0};
		glm::vec3 MaxWorkGroupSize{0};
		uint32_t  MaxWorkGroupInvocations{0};
	};

	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;

		virtual void Init() = 0;
		virtual void Render() = 0;
		virtual void Present() = 0;

		virtual void SetVSync(bool enabled) = 0;

	protected:
		ComputeInfo m_ComputeInfo;
	};
}