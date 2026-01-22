#include "kbrpch.h"
#include "D3D11RenderPass.h"

namespace Kerberos 
{
	D3D11RenderPass::D3D11RenderPass(const RenderPassSpecification& spec)
		: m_DebugName(spec.Name), m_Pipeline(spec.Pipeline)
	{
		
	}

	D3D11RenderPass::~D3D11RenderPass() = default;

	void D3D11RenderPass::Begin() 
	{
		m_Pipeline->Bind();
	}

	void D3D11RenderPass::End() 
	{
	}

	void D3D11RenderPass::SetInput(std::string_view name, const Ref<Texture2D>& texture)
	{
		KBR_CORE_ASSERT(false, "D3D11RenderPass::SetInput(Texture2D) is not yet implemented!");
	}

	void D3D11RenderPass::SetInput(std::string_view name, const Ref<UniformBuffer>& uniformBuffer)
	{
		KBR_CORE_ASSERT(false, "D3D11RenderPass::SetInput(UniformBuffer) is not yet implemented!");
	}

	Ref<Texture2D> D3D11RenderPass::GetOutputImage(uint32_t index) const
	{
		KBR_CORE_ASSERT(false, "D3D11RenderPass::GetOutputImage is not yet implemented!");
		return nullptr;
	}

	bool D3D11RenderPass::Validate() const
	{
		KBR_CORE_ASSERT(false, "D3D11RenderPass::Validate is not yet implemented!");
		return false;
	}

	void D3D11RenderPass::Bake()
	{
		KBR_CORE_ASSERT(false, "D3D11RenderPass::Bake is not yet implemented!");
	}
}