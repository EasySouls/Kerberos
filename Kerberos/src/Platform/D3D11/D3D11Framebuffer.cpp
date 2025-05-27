#include "kbrpch.h"
#include "D3D11Framebuffer.h"

namespace Kerberos
{
	D3D11Framebuffer::D3D11Framebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{
	}

	void D3D11Framebuffer::Invalidate()
	{
	}

	void D3D11Framebuffer::Bind()
	{
	}

	void D3D11Framebuffer::Unbind()
	{
	}

	void D3D11Framebuffer::Resize(uint32_t width, uint32_t height)
	{
	}
}
