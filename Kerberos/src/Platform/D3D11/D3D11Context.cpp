#include "kbrpch.h"

#include "D3D11Context.h"
#include "Kerberos/Renderer/Vertex.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <Platform/D3D11/D3D11Shader.h>

namespace Kerberos
{
	namespace Utils
	{
		static std::string GetFeatureLevelName(const D3D_FEATURE_LEVEL featureLevel)
		{
			switch (featureLevel)
			{
			case D3D_FEATURE_LEVEL_10_0: return "10.0";
			case D3D_FEATURE_LEVEL_10_1: return "10.1";
			case D3D_FEATURE_LEVEL_11_0: return "11.0";
			case D3D_FEATURE_LEVEL_11_1: return "11.1";
			case D3D_FEATURE_LEVEL_12_0: return "12.0";
			case D3D_FEATURE_LEVEL_12_1: return "12.1";
			case D3D_FEATURE_LEVEL_12_2: return "12.2";
			default: return "Unknown";
			}
		}
	}

	D3D11Context* D3D11Context::s_Instance = nullptr;

	D3D11Context::D3D11Context(GLFWwindow* windowHandle)
		: m_GlfwWindowHandle(windowHandle)
	{
		KBR_PROFILE_FUNCTION();

		KBR_CORE_ASSERT(!s_Instance, "D3D11Context already exists!");
		s_Instance = this;

		KBR_CORE_ASSERT(windowHandle, "GLFW window handle is null!");

		m_WindowHandle = glfwGetWin32Window(m_GlfwWindowHandle);

		KBR_CORE_ASSERT(m_WindowHandle, "Win32 window handle is null!");
	}

	D3D11Context::~D3D11Context()
	{
		m_DeviceContext->Flush();
		m_VertexBuffer.Reset();

		DestroySwapChainResources();
		m_SwapChain.Reset();
		m_DxgiFactory.Reset();
		m_PixelShader.Reset();
		m_VertexShader.Reset();
		
		m_DeviceContext.Reset();
#ifdef KBR_DEBUG
		m_DebugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		m_DebugDevice.Reset();
#endif
		m_Device.Reset();
		s_Instance = nullptr;
		KBR_CORE_INFO("D3D11Context destroyed successfully!");
	}

	void D3D11Context::Init()
	{
		if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&m_DxgiFactory))))
		{
			KBR_CORE_ASSERT(false, "Could not create DXGIFactory");
		}

		constexpr int width = 1280;
		constexpr int height = 720;

		DXGI_SWAP_CHAIN_DESC sd = {};
		ZeroMemory(&sd, sizeof(sd));
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

		UINT createDeviceFlags = D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef KBR_DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG; // Enable debug layer in debug builds
#endif

		D3D_FEATURE_LEVEL featureLevel;
		constexpr D3D_FEATURE_LEVEL featureLevels[] = {
			D3D_FEATURE_LEVEL_12_2,
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};
		constexpr UINT numFeatureLevels = _countof(featureLevels);

		constexpr D3D_DRIVER_TYPE driverTypes[] = {
			D3D_DRIVER_TYPE_HARDWARE,						// Hardware acceleration
			D3D_DRIVER_TYPE_WARP,							// Software fallback
			D3D_DRIVER_TYPE_REFERENCE						// Debugging purposes
		};
		constexpr UINT numDriverTypes = _countof(driverTypes);

		ComPtr<ID3D11DeviceContext> deviceContext;

		for (const auto driverType : driverTypes)
		{
			const HRESULT hr = D3D11CreateDeviceAndSwapChain(
				nullptr,
				driverType,
				nullptr,
				createDeviceFlags,
				featureLevels,
				numFeatureLevels,
				D3D11_SDK_VERSION,
				&sd,
				&m_SwapChain,
				&m_Device,
				&featureLevel,
				&deviceContext
			);
			if (SUCCEEDED(hr))
			{
				std::string featureLevelName = Utils::GetFeatureLevelName(featureLevel);

				KBR_CORE_INFO("D3D11 device created successfully with feature level: {0}", featureLevelName);
				break;
			}
		}

		if (m_Device == nullptr || m_SwapChain == nullptr || deviceContext == nullptr)
		{
			KBR_CORE_ERROR("Failed to create D3D11 device, swap chain, or device context!");
			KBR_CORE_ASSERT(false, "Failed to create D3D11 device, swap chain, or device context!");
			return;
		}

#ifdef KBR_DEBUG
		if (FAILED(m_Device.As(&m_DebugDevice)))
		{
			KBR_CORE_ERROR("Failed to create debug device!");
			return;
		}
#endif

		m_DeviceContext = deviceContext;

		CreateSwapChainResources();

		ComPtr<ID3DBlob> vertexShaderBlob = nullptr;
		m_VertexShader = D3D11Shader::CreateVertexShader(L"assets/shaders/Main.vs.hlsl", vertexShaderBlob);
		if (m_VertexShader == nullptr)
		{
			KBR_CORE_ERROR("Failed to create vertex shader!");
			return;
		}

		m_PixelShader = D3D11Shader::CreatePixelShader(L"assets/shaders/Main.ps.hlsl");
		if (m_PixelShader == nullptr)
		{
			KBR_CORE_ERROR("Failed to create pixel shader!");
			return;
		}
		
		// Create the Input Layout Descriptor
		// TODO: Use ShaderDataType to create the input layout dynamically
		constexpr D3D11_INPUT_ELEMENT_DESC vertexInputLayoutInfo[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, Position), D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, Normal), D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, TexCoord), D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		const HRESULT hr = m_Device->CreateInputLayout(
			vertexInputLayoutInfo,
			_countof(vertexInputLayoutInfo),
			vertexShaderBlob->GetBufferPointer(),
			vertexShaderBlob->GetBufferSize(),
			&m_VertexLayout);

		if (FAILED(hr))
		{
			KBR_CORE_ERROR("Failed to create vertex input layout!");
			return;
		}

		constexpr Vertex vertices[] =
		{
			{ { 0.0f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.5f, 1.0f } },
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
			{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } }
		};

		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = sizeof(vertices);
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = vertices;

		if (FAILED(m_Device->CreateBuffer(&bufferDesc, &initData, &m_VertexBuffer)))
		{
			KBR_CORE_ERROR("Failed to create vertex buffer!");
			return;
		}

	}

	void D3D11Context::SwapBuffers()
	{
		D3D11_VIEWPORT viewport;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(m_WindowWidth);
		viewport.Height = static_cast<float>(m_WindowHeight);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		constexpr float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
		constexpr UINT vertexStride = sizeof(Vertex);
		constexpr UINT vertexOffset = 0;

		m_DeviceContext->ClearRenderTargetView(m_RenderTargetView.Get(), clearColor);

		m_DeviceContext->IASetInputLayout(m_VertexLayout.Get());
		m_DeviceContext->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &vertexStride, &vertexOffset);
		m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_DeviceContext->VSSetShader(m_VertexShader.Get(), nullptr, 0);
		m_DeviceContext->RSSetViewports(1, &viewport);
		m_DeviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0);

		m_DeviceContext->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), nullptr);

		m_DeviceContext->Draw(3, 0); // Draw 3 vertices

		if (FAILED(m_SwapChain->Present(1, 0)))
		{
			KBR_CORE_ERROR("Failed to present swap chain!");
		}
	}

	void D3D11Context::OnWindowResize(const uint32_t width, const uint32_t height)
	{
		m_DeviceContext->Flush();

		DestroySwapChainResources();

		const HRESULT hr = m_SwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

		if (FAILED(hr))
		{
			KBR_CORE_ERROR("Failed to resize swapchain buffers!");
		}

		m_WindowWidth = width;
		m_WindowHeight = height;

		CreateSwapChainResources();
	}

	bool D3D11Context::CreateSwapChainResources()
	{
		ComPtr<ID3D11Texture2D> backBuffer = nullptr;
		if (FAILED(m_SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))))
		{
			KBR_CORE_ERROR("Failed to get back buffer from swap chain!");
			return false;
		}

		if (FAILED(m_Device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_RenderTargetView)))
		{
			KBR_CORE_ERROR("Failed to create render target view!");
			return false;
		}

		return true;
	}

	void D3D11Context::DestroySwapChainResources()
	{
		m_RenderTargetView.Reset();
	}
}