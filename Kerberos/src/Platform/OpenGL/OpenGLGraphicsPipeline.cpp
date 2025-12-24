#include "kbrpch.h"
#include "OpenGLGraphicsPipeline.h"

#include <glad/glad.h>

namespace Kerberos 
{
	OpenGLGraphicsPipeline::OpenGLGraphicsPipeline(PipelineSpecification spec)
		: m_Specification(std::move(spec))
	{
	}

	void OpenGLGraphicsPipeline::ApplyCullMode() const 
	{
		switch (m_Specification.CullMode)
		{
		case CullMode::None:
			glDisable(GL_CULL_FACE);
			break;
		case CullMode::Front:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			break;
		case CullMode::Back:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			break;
		case CullMode::FrontAndBack:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT_AND_BACK);
			break;
		}
	}

	void OpenGLGraphicsPipeline::ApplyDepthTest() const 
	{
		switch (m_Specification.DepthTest)
		{
		case DepthTest::None:
			glDisable(GL_DEPTH_TEST);
			break;
		case DepthTest::Less:
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			break;
		case DepthTest::LessEqual:
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			break;
		case DepthTest::Equal:
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_EQUAL);
			break;
		case DepthTest::Greater:
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_GREATER);
			break;
		case DepthTest::GreaterEqual:
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_GEQUAL);
			break;
		case DepthTest::NotEqual:
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_NOTEQUAL);
			break;
		case DepthTest::Always:
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_ALWAYS);
			break;
		case DepthTest::Never:
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_NEVER);
			break;
		}
	}

	void OpenGLGraphicsPipeline::ApplyFrontFace() const 
	{
		switch (m_Specification.FrontFace)
		{
		case WindingOrder::Clockwise:
			glFrontFace(GL_CW);
			break;
		case WindingOrder::CounterClockwise:
			glFrontFace(GL_CCW);
			break;
		}
	}

	void OpenGLGraphicsPipeline::ApplyPolygonMode() const 
	{
		if (m_Specification.Wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	void OpenGLGraphicsPipeline::Bind() const 
	{
		ApplyCullMode();

		ApplyDepthTest();

		ApplyFrontFace();

		ApplyPolygonMode();

		m_Specification.Shader->Bind();
	}
}
