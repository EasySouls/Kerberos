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

		virtual void SetInput(std::string_view name, const Ref<Texture2D>& texture) = 0;

		virtual Ref<Texture2D> GetOutputImage(uint32_t index) const = 0;

		/**
		 * @brief Checks whether the current renderpass is in a valid state.
		 * @return true if the object is valid; otherwise, false.
		 */
		virtual bool Validate() const = 0;

		/**
		 * @brief Bakes the renderpass, preparing it for execution.
		 * 
		 * Creates descriptor sets, and ensures everything is ready for rendering.
		 * Should be called after setting all inputs and outputs.
		 */
		virtual void Bake() = 0;

		static Ref<RenderPass> Create(const RenderPassSpecification& spec);
	};
}
