#include "kbrpch.h"
#include "DirectX11Context.h"
#include "Kerberos/Core.h"

namespace Kerberos
{
    DirectX11Context::DirectX11Context(const HWND windowHandle)
        : m_WindowHandle(windowHandle)
    {
        KBR_CORE_ASSERT(windowHandle, "Window handle is null!");
    }

    void DirectX11Context::Init()
    {
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        swapChainDesc.BufferCount = 1;
        swapChainDesc.BufferDesc.Width = 0;
        swapChainDesc.BufferDesc.Height = 0;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.OutputWindow = m_WindowHandle;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Windowed = TRUE;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        D3D_FEATURE_LEVEL featureLevel;
        constexpr D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            0,
            featureLevels,
            1,
            D3D11_SDK_VERSION,
            &swapChainDesc,
            &m_SwapChain,
            &m_Device,
            &featureLevel,
            &m_DeviceContext
        );

        KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create DirectX 11 device and swap chain!");

        ID3D11Texture2D* backBuffer;
        hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
        KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to get back buffer!");

        hr = m_Device->CreateRenderTargetView(backBuffer, nullptr, &m_RenderTargetView);
        KBR_CORE_ASSERT(SUCCEEDED(hr), "Failed to create render target view!");
        backBuffer->Release();

        m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, nullptr);
    }

    void DirectX11Context::SwapBuffers()
    {
        m_SwapChain->Present(1, 0);
    }
}
