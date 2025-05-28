#include "kbrpch.h"
#include "D3D11Framebuffer.h"

#include "D3D11Context.h"

namespace Kerberos
{
    namespace Utils
    {
        static DXGI_FORMAT FramebufferTextureFormatToDXGIFormat(const FramebufferTextureFormat format)
        {
            switch (format)
            {
            case FramebufferTextureFormat::RGBA8:           return DXGI_FORMAT_R8G8B8A8_UNORM;
            case FramebufferTextureFormat::DEPTH24STENCIL8: return DXGI_FORMAT_D24_UNORM_S8_UINT;
            case FramebufferTextureFormat::None:
	            break;
            }
            KBR_CORE_ASSERT(false, "Unknown Framebuffer Texture Format!");
            return DXGI_FORMAT_UNKNOWN;
        }

        static DXGI_FORMAT FramebufferTextureFormatToSRVFormat(const FramebufferTextureFormat format)
        {
            // Shader resource views might need different formats for depth/stencil
            // e.g., DXGI_FORMAT_D24_UNORM_S8_UINT for DSV becomes DXGI_FORMAT_R24_UNORM_X8_TYPELESS or DXGI_FORMAT_R24_UNORM_S8_UINT for SRV
            // Here, we'll keep it simple and assume the base format works for SRV.
            // For depth, you often create a typeless resource and then a specific format SRV.
            // For simplicity, we'll assume the DSV format is also usable as SRV for this example.
            // If you need to sample depth, adjust this.
            return FramebufferTextureFormatToDXGIFormat(format);
        }

        static bool IsDepthFormat(const FramebufferTextureFormat format)
        {
            return format == FramebufferTextureFormat::DEPTH24STENCIL8;
        }
    }

	D3D11Framebuffer::D3D11Framebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{
        m_Device = D3D11Context::Get().GetDevice();
        m_Context = D3D11Context::Get().GetDeviceContext();

        // Populate color and depth attachment specs
        for (auto format : spec.Attachments.Attachments)
        {
            if (Utils::IsDepthFormat(format.TextureFormat))
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

	D3D11Framebuffer::~D3D11Framebuffer()
	{
        ReleaseResources();
	}

	void D3D11Framebuffer::Invalidate()
	{
        ReleaseResources();

        HRESULT hr;

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
                DXGI_FORMAT textureFormat = Utils::FramebufferTextureFormatToDXGIFormat(m_ColorAttachmentSpecs[i].TextureFormat);
                DXGI_FORMAT srvFormat = Utils::FramebufferTextureFormatToSRVFormat(m_ColorAttachmentSpecs[i].TextureFormat);

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

                hr = m_Device->CreateTexture2D(&textureDesc, nullptr, m_ColorTextures[i].GetAddressOf());
                KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create color texture!");

                D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
                rtvDesc.Format = textureFormat;
                rtvDesc.ViewDimension = (m_Specification.Samples > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2D.MipSlice = 0;

                hr = m_Device->CreateRenderTargetView(m_ColorTextures[i].Get(), &rtvDesc, m_ColorRTVs[i].GetAddressOf());
                KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create RTV!");

                if (m_Specification.Samples == 1)
                {
                    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                    srvDesc.Format = srvFormat;
                    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                    srvDesc.Texture2D.MipLevels = 1;
                    srvDesc.Texture2D.MostDetailedMip = 0;

                    hr = m_Device->CreateShaderResourceView(m_ColorTextures[i].Get(), &srvDesc, m_ColorSRVs[i].GetAddressOf());
                    KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create SRV!");
                }
                else // Create resolved texture for multisampling
                {
                    D3D11_TEXTURE2D_DESC resolvedTextureDesc = textureDesc;
                    resolvedTextureDesc.SampleDesc.Count = 1;
                    resolvedTextureDesc.SampleDesc.Quality = 0;
                    resolvedTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // Only SRV for resolved texture

                    hr = m_Device->CreateTexture2D(&resolvedTextureDesc, nullptr, m_ResolvedColorTextures[i].GetAddressOf());
                    KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create resolved color texture!");

                    D3D11_SHADER_RESOURCE_VIEW_DESC resolvedSrvDesc = {};
                    resolvedSrvDesc.Format = srvFormat;
                    resolvedSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                    resolvedSrvDesc.Texture2D.MipLevels = 1;
                    resolvedSrvDesc.Texture2D.MostDetailedMip = 0;

                    hr = m_Device->CreateShaderResourceView(m_ResolvedColorTextures[i].Get(), &resolvedSrvDesc, m_ResolvedColorSRVs[i].GetAddressOf());
                    KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create resolved SRV!");
                }
            }
        }

        // --- Create Depth/Stencil Attachment ---
        if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None)
        {
            DXGI_FORMAT depthFormat = Utils::FramebufferTextureFormatToDXGIFormat(m_DepthAttachmentSpec.TextureFormat);

            D3D11_TEXTURE2D_DESC depthStencilDesc = {};
            depthStencilDesc.Width = m_Specification.Width;
            depthStencilDesc.Height = m_Specification.Height;
            depthStencilDesc.MipLevels = 1;
            depthStencilDesc.ArraySize = 1;
            depthStencilDesc.Format = depthFormat; // Use the direct format for the texture
            depthStencilDesc.SampleDesc.Count = m_Specification.Samples;
            depthStencilDesc.SampleDesc.Quality = 0;
            depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
            depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            depthStencilDesc.CPUAccessFlags = 0;
            depthStencilDesc.MiscFlags = 0;

            hr = m_Device->CreateTexture2D(&depthStencilDesc, nullptr, m_DepthTexture.GetAddressOf());
            KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create depth texture!");

            D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = depthFormat;
            dsvDesc.ViewDimension = (m_Specification.Samples > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = 0;

            hr = m_Device->CreateDepthStencilView(m_DepthTexture.Get(), &dsvDesc, m_DepthStencilView.GetAddressOf());
            KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create DSV!");
        }

        KBR_CORE_INFO("D3D11 Framebuffer Invalidate complete: {0}x{1}, Samples: {2}", m_Specification.Width, m_Specification.Height, m_Specification.Samples);
	}

	void D3D11Framebuffer::Bind()
	{
        KBR_CORE_ASSERT(m_Context, "D3DContext not initialized!");

        // Save original RTV/DSV and viewport
        m_OriginalRTV = nullptr;
        m_OriginalDSV = nullptr;
        m_OriginalNumViewports = 1; // Always query for at least one viewport
        m_Context->OMGetRenderTargets(1, m_OriginalRTV.GetAddressOf(), m_OriginalDSV.GetAddressOf());
        m_Context->RSGetViewports(&m_OriginalNumViewports, &m_OriginalViewport);

        // --- Create a temporary vector of raw pointers to pass to OMSetRenderTargets ---
        std::vector<ID3D11RenderTargetView*> rawRTVs(m_ColorRTVs.size());
        for (size_t i = 0; i < m_ColorRTVs.size(); ++i)
        {
            rawRTVs[i] = m_ColorRTVs[i].Get(); // Get the raw pointer from the ComPtr
        }

        // Bind our RTVs and DSV
        m_Context->OMSetRenderTargets(static_cast<UINT>(m_ColorRTVs.size()), rawRTVs.data(), m_DepthStencilView.Get());

        // Set the viewport
        D3D11_VIEWPORT viewport = {};
        viewport.Width = static_cast<float>(m_Specification.Width);
        viewport.Height = static_cast<float>(m_Specification.Height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        m_Context->RSSetViewports(1, &viewport);

        // Clear the render target(s) and depth/stencil
        for (auto& rtv : m_ColorRTVs)
        {
	        constexpr float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // Default clear to black
            m_Context->ClearRenderTargetView(rtv.Get(), clearColor);
        }
        if (m_DepthStencilView)
        {
            m_Context->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        }
	}

	void D3D11Framebuffer::Unbind()
	{
        KBR_CORE_ASSERT(m_Context, "D3DContext not initialized!");

        // If multisampled, resolve the textures before they are potentially read by ImGui
        if (m_Specification.Samples > 1)
        {
            for (size_t i = 0; i < m_ColorTextures.size(); ++i)
            {
                const DXGI_FORMAT format = Utils::FramebufferTextureFormatToDXGIFormat(m_ColorAttachmentSpecs[i].TextureFormat);
                m_Context->ResolveSubresource(m_ResolvedColorTextures[i].Get(), 0, m_ColorTextures[i].Get(), 0, format);
            }
        }

        // Restore original render targets and viewport
        m_Context->OMSetRenderTargets(1, &m_OriginalRTV, m_OriginalDSV.Get());
        m_Context->RSSetViewports(m_OriginalNumViewports, &m_OriginalViewport);

        // Release the temporary references to the original RTV/DSV
        if (m_OriginalRTV) m_OriginalRTV->Release();
        if (m_OriginalDSV) m_OriginalDSV->Release();
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
            // No actual resize needed
            return;
        }

        m_Specification.Width = width;
        m_Specification.Height = height;

        Invalidate(); // Recreate all resources with new dimensions
	}

	uint32_t D3D11Framebuffer::GetColorAttachmentRendererID(const uint32_t index) const
	{
        KBR_CORE_ASSERT(index < m_ColorAttachmentSpecs.size(), "Index out of bounds for color attachment!");
        if (m_Specification.Samples > 1)
        {
            /// For multisampled, return the SRV of the resolved texture
            KBR_CORE_ASSERT(index < m_ResolvedColorSRVs.size(), "Resolved SRV index out of bounds!");
            return reinterpret_cast<intptr_t>(m_ResolvedColorSRVs[index].Get());
        }

        /// For non-multisampled, return the SRV of the main texture
        KBR_CORE_ASSERT(index < m_ColorSRVs.size(), "SRV index out of bounds!");
        return reinterpret_cast<intptr_t>(m_ColorSRVs[index].Get());
	}

	void D3D11Framebuffer::ReleaseResources() const 
    {
        for (auto & rtv : m_ColorRTVs) if (rtv) rtv->Release();
        for (auto& srv : m_ColorSRVs) if (srv) srv->Release(); // This would be for non-multisampled textures directly
        for (auto& tex : m_ColorTextures) if (tex) tex->Release();

        for (auto& srv : m_ResolvedColorSRVs) if (srv) srv->Release();
        for (auto& tex : m_ResolvedColorTextures) if (tex) tex->Release();

        if (m_DepthStencilView) m_DepthStencilView->Release();
        if (m_DepthTexture) m_DepthTexture->Release();
	}
}
