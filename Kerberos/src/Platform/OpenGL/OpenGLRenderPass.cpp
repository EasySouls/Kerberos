#include "kbrpch.h"
#include "OpenGLRenderPass.h"

namespace Kerberos
{
	OpenGLRenderPass::OpenGLRenderPass(RenderPassSpecification spec)
		: m_Specification(std::move(spec))
	{
	}

	void OpenGLRenderPass::Begin() 
	{
		m_Specification.Pipeline->Bind();
	}

	void OpenGLRenderPass::End() 
	{
	}

	void OpenGLRenderPass::SetInput(std::string_view name, const Ref<Texture2D>& texture)
	{
		KBR_CORE_ASSERT(false, "OpenGLRenderPass::SetInput is not yet implemented!");
	}

	void OpenGLRenderPass::SetInput(std::string_view name, const Ref<UniformBuffer>& uniformBuffer) 
	{
		KBR_CORE_ASSERT(false, "OpenGLRenderPass::SetInput is not yet implemented!");
	}

	void OpenGLRenderPass::SetInput(std::string_view name, const Ref<TextureCube>& texture) 
	{
		KBR_CORE_ASSERT(false, "OpenGLRenderPass::SetInput is not yet implemented!");
	}

	Ref<Texture2D> OpenGLRenderPass::GetOutputImage(uint32_t index) const
	{
		KBR_CORE_ASSERT(false, "OpenGLRenderPass::GetOutputImage is not yet implemented!");
		return nullptr;
	}

	Ref<Texture2D> OpenGLRenderPass::GetOutputDepthImage() const 
	{
		KBR_CORE_ASSERT(false, "OpenGLRenderPass::GetOutputDepthImage is not yet implemented!");
		return nullptr;
	}

	bool OpenGLRenderPass::Validate() const
	{
		KBR_CORE_ASSERT(false, "OpenGLRenderPass::Validate is not yet implemented!");
		return false;
	}

	void OpenGLRenderPass::Bake()
	{
		KBR_CORE_ASSERT(false, "OpenGLRenderPass::Bake is not yet implemented!");
	}
}