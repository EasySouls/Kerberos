#pragma once
#include "Kerberos/Renderer/UniformBuffer.h"

namespace Kerberos
{
	class VulkanUniformBuffer : public UniformBuffer
	{
	public:
		VulkanUniformBuffer(uint32_t size, uint32_t binding);
		~VulkanUniformBuffer() override;

		void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

		void SetDebugName(const std::string& debugName) override;
	};
}
