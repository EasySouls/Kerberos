#pragma once

#include "Kerberos/Renderer/GraphicsContext.h"
#include <d3d11.h>

namespace Kerberos
{
	class D3D11Context : public GraphicsContext
	{
	public:
		D3D11Context(HWND windowHandle);
		~D3D11Context();

		void Init() override;
		void SwapBuffers() override;
	private:
		HWND m_WindowHandle;
		ID3D11Device* m_Device;
		ID3D11DeviceContext* m_DeviceContext;
		IDXGISwapChain* m_SwapChain;
		ID3D11RenderTargetView* m_RenderTargetView;
	};
}