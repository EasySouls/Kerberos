#include "kbrpch.h"

#include "D3D11Context.h"

namespace Kerberos
{
	D3D11Context::D3D11Context(const HWND windowHandle)
		: m_WindowHandle(windowHandle)
	{
		KBR_CORE_ASSERT(windowHandle, "Window handle is null!");
	}

	D3D11Context::~D3D11Context()
	{
		if (m_SwapChain)
		{
			m_SwapChain->Release();
		}
		if (m_Device)
		{
			m_Device->Release();
		}
		if (m_DeviceContext)
		{
			m_DeviceContext->Release();
		}
		if (m_RenderTargetView)
		{
			m_RenderTargetView->Release();
		}
	}

	void D3D11Context::Init()
	{
		constexpr int width = 1280;
		constexpr int height = 720;

		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferCount = 1;									// One back buffer
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // Use 32-bit color
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;   // How the back buffer will be used
		sd.OutputWindow = m_WindowHandle;
		sd.SampleDesc.Count = 4;							// MSAA
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;			// Discard old frames

		constexpr UINT createDeviceFlags = 0;

		D3D_FEATURE_LEVEL featureLevel;
		constexpr D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

		const HRESULT hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			createDeviceFlags,
			featureLevels,
			0,
			D3D11_SDK_VERSION,
			&sd,
			&m_SwapChain,
			&m_Device,
			&featureLevel,
			&m_DeviceContext
		);

		if (FAILED(hr))
		{
			KBR_CORE_ERROR("Failed to create device and swap chain!");
			return;
		}

		ID3D11Resource* backBuffer = nullptr;
		m_SwapChain->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(&backBuffer));
		m_Device->CreateRenderTargetView(backBuffer, nullptr, &m_RenderTargetView);
		backBuffer->Release();
	}

	void D3D11Context::SwapBuffers()
	{
		m_SwapChain->Present(1, 0);
	}
}