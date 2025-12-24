#pragma once

#include "Kerberos/Renderer/GraphicsPipeline.h"

namespace Kerberos
{
	class OpenGLGraphicsPipeline : public GraphicsPipeline
	{
	public:
		explicit OpenGLGraphicsPipeline(PipelineSpecification spec);
		~OpenGLGraphicsPipeline() override = default;

		void Bind() const override;
		const PipelineSpecification& GetSpecification() const override { return m_Specification; }

	private:
		void ApplyCullMode() const;
		void ApplyDepthTest() const;
		void ApplyFrontFace() const;
		void ApplyPolygonMode() const;

	private:
		PipelineSpecification m_Specification;
	};
}