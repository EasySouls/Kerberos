#pragma once

#include "Kerberos/Renderer/UniformBuffer.h"

namespace Kerberos 
{
	class OpenGLUniformBuffer : public UniformBuffer 
	{
	public:
		OpenGLUniformBuffer(uint32_t size, uint32_t binding);
		~OpenGLUniformBuffer() override;

		void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

		void SetDebugName(const std::string& debugName) override;

	private:
		uint32_t m_RendererID = 0;
	};
}