#pragma once

#include "Pipeline.h"

namespace Kerberos
{
	class RenderPass
	{
	public:
		struct RenderPassSpecification
		{
			std::string Name;
			Ref<Pipeline> Pipeline;
		};

		virtual ~RenderPass() = default;
		static Ref<RenderPass> Create(const RenderPassSpecification& spec);
	};
}
