#pragma once

#include "Kerberos/Renderer/Framebuffer.h"

#include <d3d11.h>
#include <wrl.h>

namespace Kerberos
{
	using Microsoft::WRL::ComPtr;

	class D3D11Framebuffer final : public Framebuffer
	{
	public:
		explicit D3D11Framebuffer(const FramebufferSpecification& spec);
		~D3D11Framebuffer() override = default;

		void Invalidate();

		void Bind() override;
		void Unbind() override;

		void Resize(uint32_t width, uint32_t height) override;
		int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		void BindColorTexture(uint32_t slot, uint32_t index) const override;
		void BindDepthTexture(uint32_t slot) const override;

		void ClearAttachment(uint32_t attachmentIndex, int value) override;
		void ClearDepthAttachment(float value) const override;

		uint64_t GetColorAttachmentRendererID(uint32_t index = 0) const override;
		uint64_t GetDepthAttachmentRendererID() const override;

		FramebufferSpecification& GetSpecification() override { return m_Specification; }
		const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

		void SetDebugName(const std::string& name) const override;

	private:
		void ReleaseResources() const;

	private:
		FramebufferSpecification m_Specification;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecs;
		FramebufferTextureSpecification m_DepthAttachmentSpec = FramebufferTextureFormat::None;

		std::vector<ComPtr<ID3D11Texture2D>> m_ColorTextures;
		std::vector<ComPtr<ID3D11RenderTargetView>> m_ColorRTVs;
		std::vector<ComPtr<ID3D11ShaderResourceView>> m_ColorSRVs;

		// For multisampled textures, we need a separate set of textures
	   // to resolve to, as ImGui typically expects non-multisampled textures.
		std::vector<ComPtr<ID3D11Texture2D>> m_ResolvedColorTextures;
		std::vector<ComPtr<ID3D11ShaderResourceView>> m_ResolvedColorSRVs; // SRVs for ImGui (for multisampled, these are returned)

		ComPtr<ID3D11Texture2D> m_DepthTexture;
		ComPtr<ID3D11DepthStencilView> m_DepthStencilView;

		// Store original D3D state to restore on Unbind
		ComPtr<ID3D11RenderTargetView> m_OriginalRTV = nullptr;
		ComPtr<ID3D11DepthStencilView> m_OriginalDSV = nullptr;
		D3D11_VIEWPORT m_OriginalViewport = {};
		UINT m_OriginalNumViewports = 1;
	};
}