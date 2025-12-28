#include "kbrpch.h"
#include "D3D11RenderPass.h"

namespace Kerberos 
{
	D3D11RenderPass::D3D11RenderPass(const RenderPassSpecification& spec)
		: RenderPass()
	{
		KBR_CORE_ASSERT(false, "D3D11RenderPass is not yet implemented!");
	}

	D3D11RenderPass::~D3D11RenderPass()
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