#include "kbrpch.h"
#include "D3D11Framebuffer.h"

#include "D3D11Context.h"
#include "Utils/TextureUtils.h"

namespace Kerberos
{
	D3D11Framebuffer::D3D11Framebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{
        // Populate color and depth attachment specs
        for (auto& format : spec.Attachments.Attachments)
        {
            if (IsDepthFormat(format.TextureFormat))
            {
                m_DepthAttachmentSpec = format;
            }
            else
            {
                m_ColorAttachmentSpecs.emplace_back(format);
            }
        }

        Invalidate();
	}

	void D3D11Framebuffer::Invalidate()
	{
        ReleaseResources();

        HRESULT hr;

        auto device = D3D11Context::Get().GetDevice();

        /// Create Color Attachments
        if (!m_ColorAttachmentSpecs.empty())
        {
            m_ColorTextures.resize(m_ColorAttachmentSpecs.size(), nullptr);
            m_ColorRTVs.resize(m_ColorAttachmentSpecs.size(), nullptr);

            // Only create SRVs for non-multisampled textures directly
            // For multisampled, the resolved textures will have SRVs
            if (m_Specification.Samples == 1)
            {
                m_ColorSRVs.resize(m_ColorAttachmentSpecs.size(), nullptr);
            }
            else
            {
                m_ResolvedColorTextures.resize(m_ColorAttachmentSpecs.size(), nullptr);
                m_ResolvedColorSRVs.resize(m_ColorAttachmentSpecs.size(), nullptr);
            }

            for (size_t i = 0; i < m_ColorAttachmentSpecs.size(); ++i)
            {
                DXGI_FORMAT textureFormat = TextureUtils::GetTextureFormat(m_ColorAttachmentSpecs[i].TextureFormat);
                DXGI_FORMAT srvFormat = TextureUtils::GetSRVFormat(m_ColorAttachmentSpecs[i].TextureFormat);

                D3D11_TEXTURE2D_DESC textureDesc = {};
                textureDesc.Width = m_Specification.Width;
                textureDesc.Height = m_Specification.Height;
                textureDesc.MipLevels = 1;
                textureDesc.ArraySize = 1;
                textureDesc.Format = textureFormat;
                textureDesc.SampleDesc.Count = m_Specification.Samples;
                textureDesc.SampleDesc.Quality = 0; // Or D3DContext::Get()->GetMSAASampleQuality if exposed
                textureDesc.Usage = D3D11_USAGE_DEFAULT;
                textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
                if (m_Specification.Samples == 1)
                { // If not multisampled, we can bind directly as SRV
                    textureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
                }
                textureDesc.CPUAccessFlags = 0;
                textureDesc.MiscFlags = 0;

                hr = device->CreateTexture2D(&textureDesc, nullptr, m_ColorTextures[i].GetAddressOf());
                KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create color texture!");

                D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
                rtvDesc.Format = TextureUtils::GetRTVFormat(m_ColorAttachmentSpecs[i].TextureFormat);
                rtvDesc.ViewDimension = (m_Specification.Samples > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2D.MipSlice = 0;

                hr = device->CreateRenderTargetView(m_ColorTextures[i].Get(), &rtvDesc, m_ColorRTVs[i].GetAddressOf());
                KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create RTV!");

                if (m_Specification.Samples == 1)
                {
                    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                    srvDesc.Format = srvFormat;
                    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                    srvDesc.Texture2D.MipLevels = 1;
                    srvDesc.Texture2D.MostDetailedMip = 0;

                    hr = device->CreateShaderResourceView(m_ColorTextures[i].Get(), &srvDesc, m_ColorSRVs[i].GetAddressOf());
                    KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create SRV!");
                }
                else // Create resolved texture for multisampling
                {
                    D3D11_TEXTURE2D_DESC resolvedTextureDesc = textureDesc;
                    resolvedTextureDesc.SampleDesc.Count = 1;
                    resolvedTextureDesc.SampleDesc.Quality = 0;
                    resolvedTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // Only SRV for resolved texture

                    hr = device->CreateTexture2D(&resolvedTextureDesc, nullptr, m_ResolvedColorTextures[i].GetAddressOf());
                    KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create resolved color texture!");

                    D3D11_SHADER_RESOURCE_VIEW_DESC resolvedSrvDesc = {};
                    resolvedSrvDesc.Format = srvFormat;
                    resolvedSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                    resolvedSrvDesc.Texture2D.MipLevels = 1;
                    resolvedSrvDesc.Texture2D.MostDetailedMip = 0;

                    hr = device->CreateShaderResourceView(m_ResolvedColorTextures[i].Get(), &resolvedSrvDesc, m_ResolvedColorSRVs[i].GetAddressOf());
                    KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create resolved SRV!");
                }
            }
        }

        // --- Create Depth/Stencil Attachment ---
        if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None)
        {
			const DXGI_FORMAT depthTextureFormat = TextureUtils::GetTextureFormat(m_DepthAttachmentSpec.TextureFormat);
            const DXGI_FORMAT depthDsvFormat = TextureUtils::GetDSVFormat(m_DepthAttachmentSpec.TextureFormat);
			const DXGI_FORMAT depthSrvFormat = TextureUtils::GetSRVFormat(m_DepthAttachmentSpec.TextureFormat);

            D3D11_TEXTURE2D_DESC depthStencilDesc = {};
            depthStencilDesc.Width = m_Specification.Width;
            depthStencilDesc.Height = m_Specification.Height;
            depthStencilDesc.MipLevels = 1;
            depthStencilDesc.ArraySize = 1;
            depthStencilDesc.Format = depthTextureFormat;
            depthStencilDesc.SampleDesc.Count = m_Specification.Samples;
            depthStencilDesc.SampleDesc.Quality = 0;
            depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
            depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
            depthStencilDesc.CPUAccessFlags = 0;
            depthStencilDesc.MiscFlags = 0;

            hr = device->CreateTexture2D(&depthStencilDesc, nullptr, m_DepthTexture.GetAddressOf());
            KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create depth texture!");

            D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = depthDsvFormat;
            dsvDesc.ViewDimension = (m_Specification.Samples > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = 0;

            hr = device->CreateDepthStencilView(m_DepthTexture.Get(), &dsvDesc, m_DepthStencilView.GetAddressOf());
            KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create DSV!");

			D3D11_SHADER_RESOURCE_VIEW_DESC depthSrvDesc = {};
			depthSrvDesc.Format = depthSrvFormat;
			depthSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			depthSrvDesc.Texture2D.MipLevels = 1;

			hr = device->CreateShaderResourceView(m_DepthTexture.Get(), &depthSrvDesc, m_DepthSRV.GetAddressOf());
			KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create Depth SRV!");
        }

        KBR_CORE_INFO("D3D11 Framebuffer Invalidate complete: {0}x{1}, Samples: {2}", m_Specification.Width, m_Specification.Height, m_Specification.Samples);
	}

	void D3D11Framebuffer::Bind()
	{
		const auto deviceContext = D3D11Context::Get().GetImmediateContext();

        KBR_CORE_ASSERT(deviceContext, "D3DContext not initialized!");

        std::vector<ID3D11RenderTargetView*> rtvPointers;
        rtvPointers.reserve(m_ColorRTVs.size());
        for (const auto& comPtrRtv : m_ColorRTVs)
        {
            rtvPointers.push_back(comPtrRtv.Get());
        }

        /// Bind our RTVs and DSV
        deviceContext->OMSetRenderTargets(
            static_cast<uint32_t>(
                rtvPointers.size()),
            rtvPointers.data(), 
            m_DepthStencilView.Get());

        /// Set the viewport
        D3D11_VIEWPORT viewport;
        viewport.Width = static_cast<float>(m_Specification.Width);
        viewport.Height = static_cast<float>(m_Specification.Height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        deviceContext->RSSetViewports(1, &viewport);

		/// Clear is handled by D3D11RendererApi (although for now it clears only the first color attachment)
        ///// Clear the render target(s) and depth/stencil
        //for (auto& rtv : m_ColorRTVs)
        //{
	       // constexpr float clearColor[4] = { 0.0f, 1.0f, 0.0f, 1.0f }; // Default clear to green
        //    deviceContext->ClearRenderTargetView(rtv.Get(), clearColor);
        //}
        //if (m_DepthStencilView)
        //{
        //    deviceContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        //}
	}

	void D3D11Framebuffer::Unbind()
	{
		const auto deviceContext = D3D11Context::Get().GetImmediateContext();

        KBR_CORE_ASSERT(deviceContext, "D3DContext not initialized!");

        // If multisampled, resolve the textures before they are potentially read by ImGui
        if (m_Specification.Samples > 1)
        {
            for (size_t i = 0; i < m_ColorTextures.size(); ++i)
            {
                // TODO: check if this resolves to the correct format and subresource
                const DXGI_FORMAT format = TextureUtils::GetTextureFormat(m_ColorAttachmentSpecs[i].TextureFormat);
                deviceContext->ResolveSubresource(m_ResolvedColorTextures[i].Get(), 0, m_ColorTextures[i].Get(), 0, format);
            }
        }

        // Clear all shader resource views to prevent conflicts with subsequent rendering (like ImGui)
        // This is necessary because ImGui's D3D11 backend only backs up/restores slot 0
        ID3D11ShaderResourceView* nullSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = { nullptr };
        deviceContext->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullSRVs);
        deviceContext->VSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullSRVs);

        auto backBufferRTV = D3D11Context::Get().GetRenderTargetView();
        deviceContext->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), nullptr);
	}

	void D3D11Framebuffer::Resize(uint32_t width, uint32_t height)
	{
        if (width == 0 || height == 0)
        {
            KBR_CORE_WARN("Attempted to resize D3D11 framebuffer to {0}, {1}", width, height);
            return;
        }
        if (width == m_Specification.Width && height == m_Specification.Height)
        {
            /// No actual resize needed
            return;
        }

        m_Specification.Width = width;
        m_Specification.Height = height;

        /// Recreate all resources with new dimensions
        Invalidate(); 
	}

	int D3D11Framebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y) 
    {
		const auto& device = D3D11Context::Get().GetDevice();
		KBR_CORE_ASSERT(device, "D3DContext not initialized!");
		KBR_CORE_ASSERT(attachmentIndex < m_ColorAttachmentSpecs.size(), "Index out of bounds for color attachment!");

		const auto& texture = m_ColorTextures[attachmentIndex];

		const auto width = m_Specification.Width;
		const auto height = m_Specification.Height;

        D3D11_TEXTURE2D_DESC stagingDesc;
        stagingDesc.Width = width;
        stagingDesc.Height = height;
        stagingDesc.MipLevels = 1;
        stagingDesc.ArraySize = 1;
        stagingDesc.Format = DXGI_FORMAT_R32_SINT;
        stagingDesc.SampleDesc.Count = 1;
		stagingDesc.SampleDesc.Quality = 0;
        stagingDesc.BindFlags = 0;
		stagingDesc.MiscFlags = 0;
        stagingDesc.Usage = D3D11_USAGE_STAGING;
        stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

        ID3D11Texture2D* stagingTexture;
        HRESULT hr = device->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);
		KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create staging texture for ReadPixel!");

        D3D11_BOX box{};
        box.left = x;
        box.top = y;
        box.front = 0;
        box.right = x + 1;
        box.bottom = y + 1;
        box.back = 1;

		const auto& context = D3D11Context::Get().GetImmediateContext();

        context->CopySubresourceRegion(
            stagingTexture,
            0,
            0, 0, 0,
            texture.Get(),
            0,
            &box
        );

        D3D11_MAPPED_SUBRESOURCE mapped{};
        hr = context->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &mapped);
		KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to map staging texture for ReadPixel!");

        const int pixel = *static_cast<int*>(mapped.pData);

        context->Unmap(stagingTexture, 0);

        return pixel;
    }

	void D3D11Framebuffer::BindColorTexture(const uint32_t slot, const uint32_t index) const
	{
		const auto& deviceContext = D3D11Context::Get().GetImmediateContext();
        KBR_CORE_ASSERT(deviceContext, "D3DContext not initialized!");
        KBR_CORE_ASSERT(index < m_ColorAttachmentSpecs.size(), "Index out of bounds for color attachment!");

        ID3D11ShaderResourceView* srv = nullptr;
        if (m_Specification.Samples > 1)
        {
            KBR_CORE_ASSERT(index < m_ResolvedColorSRVs.size(), "Resolved SRV index out of bounds!");
            srv = m_ResolvedColorSRVs[index].Get();
        }
        else
        {
            KBR_CORE_ASSERT(index < m_ColorSRVs.size(), "SRV index out of bounds!");
            srv = m_ColorSRVs[index].Get();
        }
		deviceContext->PSSetShaderResources(slot, 1, &srv);
	}

	void D3D11Framebuffer::BindDepthTexture(const uint32_t slot) const
	{
		const auto deviceContext = D3D11Context::Get().GetImmediateContext();
		KBR_CORE_ASSERT(deviceContext, "D3DContext not initialized!");

		ID3D11ShaderResourceView* srv = m_DepthSRV.Get();
		deviceContext->PSSetShaderResources(slot, 1, &srv);
	}

	void D3D11Framebuffer::ClearAttachment(const uint32_t attachmentIndex, const int value) 
    {
        const auto& deviceContext = D3D11Context::Get().GetImmediateContext();
		KBR_CORE_ASSERT(attachmentIndex < m_ColorRTVs.size(), "Index out of bounds for color attachment!");
        KBR_CORE_ASSERT(deviceContext, "D3DContext not initialized!");

        float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        clearColor[0] = static_cast<float>(value);
		deviceContext->ClearRenderTargetView(m_ColorRTVs[attachmentIndex].Get(), clearColor);
    }

	void D3D11Framebuffer::ClearDepthAttachment(const float value) const 
    {
        const auto& deviceContext = D3D11Context::Get().GetImmediateContext();
        KBR_CORE_ASSERT(deviceContext, "D3DContext not initialized!");
		KBR_CORE_ASSERT(m_DepthStencilView, "No depth stencil view to clear!");

		deviceContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH, value, 0);
    }

	uint64_t D3D11Framebuffer::GetColorAttachmentRendererID(const uint32_t index) const
	{
        KBR_CORE_ASSERT(index < m_ColorAttachmentSpecs.size(), "Index out of bounds for color attachment!");
        if (m_Specification.Samples > 1)
        {
            /// For multisampled, return the SRV of the resolved texture
            KBR_CORE_ASSERT(index < m_ResolvedColorSRVs.size(), "Resolved SRV index out of bounds!");

			const auto& srv = m_ResolvedColorSRVs[index];
            D3D11_SHADER_RESOURCE_VIEW_DESC d;
            srv->GetDesc(&d);
            KBR_CORE_ASSERT(d.Format == DXGI_FORMAT_R32_FLOAT || d.Format == DXGI_FORMAT_R8G8B8A8_UNORM, "The SRV format cannot be sampled by ImGui");

            return reinterpret_cast<uint64_t>(srv.Get());
        }

        /// For non-multisampled, return the SRV of the main texture
        KBR_CORE_ASSERT(index < m_ColorSRVs.size(), "SRV index out of bounds!");

        const auto& srv = m_ColorSRVs[index];
        D3D11_SHADER_RESOURCE_VIEW_DESC d;
        srv->GetDesc(&d);
        KBR_CORE_ASSERT(d.Format == DXGI_FORMAT_R32_FLOAT || d.Format == DXGI_FORMAT_R8G8B8A8_UNORM, "The SRV format cannot be sampled by ImGui");

        return reinterpret_cast<uint64_t>(srv.Get());
	}

	uint64_t D3D11Framebuffer::GetDepthAttachmentRendererID() const 
    {
        const auto& srv = m_DepthSRV;
        D3D11_SHADER_RESOURCE_VIEW_DESC d;
        srv->GetDesc(&d);
        KBR_CORE_ASSERT(d.Format == DXGI_FORMAT_R32_FLOAT || d.Format == DXGI_FORMAT_R8G8B8A8_UNORM, "The SRV format cannot be sampled by ImGui");

		return reinterpret_cast<uint64_t>(srv.Get());
    }

	void D3D11Framebuffer::SetDebugName(const std::string& name) const 
    {
		// TODO: Implement setting debug name for D3D11 resources
    }

	void D3D11Framebuffer::ReleaseResources() 
    {
        for (auto & rtv : m_ColorRTVs) if (rtv) rtv.Reset();
        for (auto& srv : m_ColorSRVs) if (srv) srv.Reset(); // This would be for non-multisampled textures directly
        for (auto& tex : m_ColorTextures) if (tex) tex.Reset();

        for (auto& srv : m_ResolvedColorSRVs) if (srv) srv.Reset();
        for (auto& tex : m_ResolvedColorTextures) if (tex) tex.Reset();

        if (m_DepthStencilView) m_DepthStencilView.Reset();
        if (m_DepthTexture) m_DepthTexture.Reset();
		if (m_DepthSRV) m_DepthSRV.Reset();
	}
}
