#include "kbrpch.h"

#include "D3D11Context.h"

namespace Kerberos
{
	D3D11Context::D3D11Context(HWND windowHandle)
		: m_WindowHandle(windowHandle)
	{}

	D3D11Context::~D3D11Context()
	{}

	void D3D11Context::Init()
	{
		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferCount = 1;
		sd.BufferDesc.Width = 0;
		sd.BufferDesc.Height = 0;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = m_WindowHandle;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;
		UINT createDeviceFlags = 0;
	}
}