#pragma once

#include "Kerberos/Renderer/GraphicsContext.h"
#include <d3d11.h>
#include <dxgi1_3.h>
#include <wrl.h>
#include <GLFW/glfw3.h>

namespace Kerberos
{
	using Microsoft::WRL::ComPtr;

	class D3D11Context : public GraphicsContext
	{
	public:
		D3D11Context(GLFWwindow* windowHandle);
		~D3D11Context();

		void Init() override;
		void SwapBuffers() override;

		void OnWindowResize(uint32_t width, uint32_t height);

		static D3D11Context& Get() { return *s_Instance; }

	private:
		bool CreateSwapChainResources();
		void DestroySwapChainResources();

	private:
		GLFWwindow* m_GlfwWindowHandle = nullptr;
		HWND m_WindowHandle = nullptr;

		ComPtr<ID3D11Device> m_Device = nullptr;
		ComPtr<IDXGIFactory2> m_DxgiFactory = nullptr;
		ComPtr<ID3D11DeviceContext> m_DeviceContext = nullptr;
		ComPtr<IDXGISwapChain> m_SwapChain = nullptr;
		ComPtr<ID3D11RenderTargetView> m_RenderTargetView = nullptr;

		uint32_t m_WindowWidth = 0;
		uint32_t m_WindowHeight = 0;

		static D3D11Context* s_Instance;
	};
}