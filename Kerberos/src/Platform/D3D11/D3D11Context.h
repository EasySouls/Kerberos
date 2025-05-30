#pragma once

#include "Kerberos/Renderer/GraphicsContext.h"
#include <d3d11.h>
#include <dxgi1_3.h>
#include <wrl.h>
#ifdef KBR_DEBUG
#include <d3d11sdklayers.h>
#endif
#include <GLFW/glfw3.h>

namespace Kerberos
{
	using Microsoft::WRL::ComPtr;

	class D3D11Context final : public GraphicsContext
	{
	public:
		explicit D3D11Context(GLFWwindow* windowHandle);
		virtual ~D3D11Context();

		void Init() override;
		void SwapBuffers() override;

		void OnWindowResize(uint32_t width, uint32_t height);

		[[nodiscard]] 
		ComPtr<ID3D11Device> GetDevice() const { return m_Device; }
		[[nodiscard]] 
		ComPtr<ID3D11DeviceContext> GetImmediateContext() const { return m_ImmediateContext; }
		[[nodiscard]] 
		ComPtr<IDXGISwapChain> GetSwapChain() const { return m_SwapChain; }
		[[nodiscard]] 
		ComPtr<ID3D11RenderTargetView> GetRenderTargetView() const { return m_BackBufferRTV; }

		static D3D11Context& Get() { return *s_Instance; }

	private:
		bool CreateSwapChainResources();
		void DestroySwapChainResources();
		void ProcessInfoQueueMessages() const;

	private:
		GLFWwindow* m_GlfwWindowHandle = nullptr;
		HWND m_WindowHandle = nullptr;

		ComPtr<ID3D11Device> m_Device = nullptr;
		ComPtr<IDXGIFactory2> m_DxgiFactory = nullptr;
		ComPtr<ID3D11DeviceContext> m_ImmediateContext = nullptr;
		ComPtr<IDXGISwapChain> m_SwapChain = nullptr;
		ComPtr<ID3D11RenderTargetView> m_BackBufferRTV = nullptr;
		D3D11_VIEWPORT m_Viewport = {};
#ifdef KBR_DEBUG
		ComPtr<ID3D11Debug> m_DebugDevice = nullptr;
		ComPtr<ID3D11InfoQueue> m_InfoQueue = nullptr;
#endif

		ComPtr<ID3D11VertexShader> m_VertexShader = nullptr;
		ComPtr<ID3D11PixelShader> m_PixelShader = nullptr;
		ComPtr<ID3D11InputLayout> m_VertexLayout = nullptr;
		ComPtr<ID3D11Buffer> m_VertexBuffer = nullptr;

		uint32_t m_WindowWidth = 0;
		uint32_t m_WindowHeight = 0;

		static D3D11Context* s_Instance;
	};
}