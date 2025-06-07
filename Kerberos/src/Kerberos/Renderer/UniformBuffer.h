#pragma once

#include "Kerberos/Core.h"

namespace Kerberos 
{
	class UniformBuffer
	{
	public:
		virtual ~UniformBuffer() = default;
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;

		template<typename T>
		T& As()
		{
			return *static_cast<T*>(this);
		}

		static Ref<UniformBuffer> Create(uint32_t size, uint32_t binding);
	};
}

