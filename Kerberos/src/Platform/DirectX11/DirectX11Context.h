#pragma once 

#include "kbrpch.h"
#include "Kerberos/Renderer/GraphicsContext.h"
#include <d3d11.h>

namespace Kerberos
{
    class DirectX11Context : public GraphicsContext
    {
    public:
	    explicit DirectX11Context(HWND windowHandle);

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