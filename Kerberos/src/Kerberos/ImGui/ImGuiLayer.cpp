#include "kbrpch.h"
#include "ImGuiLayer.h"
#include "Kerberos/Application.h"

#include <imgui/imgui.h>
#define IMGUI_IMPL_API
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#ifdef KBR_PLATFORM_WINDOWS
#include <imgui/backends/imgui_impl_dx11.h>
#include "Platform/D3D11/D3D11Context.h"
#endif
#include <GLFW/glfw3.h>
#include <ImGuizmo.h>

#include "Kerberos/Renderer/Renderer.h"
#include "Kerberos/Renderer/RendererAPI.h"

namespace Kerberos
{
	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{}

	ImGuiLayer::~ImGuiLayer() = default;

	void ImGuiLayer::OnAttach()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		/*io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos; */

		io.Fonts->AddFontFromFileTTF("assets/fonts/Inter/Inter_18pt-Italic.ttf", 18.0f);
		io.Fonts->AddFontFromFileTTF("assets/fonts/Inter/Inter_18pt-Bold.ttf", 18.0f);
		io.Fonts->AddFontFromFileTTF("assets/fonts/Inter/Inter_18pt-SemiBold.ttf", 18.0f);
		ImFont* interRegular = io.Fonts->AddFontFromFileTTF("assets/fonts/Inter/Inter_18pt-Regular.ttf", 18.0f);
		io.FontDefault = interRegular;

		// Setup color style
		ImGui::StyleColorsDark();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		const Application& application = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(application.GetWindow().GetNativeWindow());


		// Setup Platform/Renderer bindings
		if (Renderer::GetAPI() == RendererAPI::API::OpenGL)
		{
			ImGui_ImplGlfw_InitForOpenGL(window, true);
			ImGui_ImplOpenGL3_Init("#version 410");
		}
		else if (Renderer::GetAPI() == RendererAPI::API::D3D11)
		{
			ImGui_ImplGlfw_InitForOther(window, true);
#ifdef KBR_PLATFORM_WINDOWS
			ImGui_ImplDX11_Init(D3D11Context::Get().GetDevice().Get(), D3D11Context::Get().GetDeviceContext().Get());
#endif
		}
	}

	void ImGuiLayer::OnDetach()
	{
		if (Renderer::GetAPI() == RendererAPI::API::OpenGL)
		{
			ImGui_ImplOpenGL3_Shutdown();
		}
		else if (Renderer::GetAPI() == RendererAPI::API::D3D11)
		{
#ifdef KBR_PLATFORM_WINDOWS
			ImGui_ImplDX11_Shutdown();
#endif
		}

		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnEvent(Event& event) 
	{
		if (m_BlockEvents)
		{
			const ImGuiIO& io = ImGui::GetIO();
			event.Handled |= event.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			event.Handled |= event.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
		}
	}

	void ImGuiLayer::OnImGuiRender()
	{
		//static bool show = false;
		//ImGui::ShowDemoWindow(&show);
	}

	void ImGuiLayer::Begin()
	{
		if (Renderer::GetAPI() == RendererAPI::API::OpenGL)
		{
			ImGui_ImplOpenGL3_NewFrame();
		}
		else if (Renderer::GetAPI() == RendererAPI::API::D3D11)
		{
#ifdef KBR_PLATFORM_WINDOWS
			ImGui_ImplDX11_NewFrame();
#endif
		}
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGuizmo::BeginFrame();
	}

	void ImGuiLayer::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		const Application& application = Application::Get();
		io.DisplaySize = ImVec2(static_cast<float>(application.GetWindow().GetWidth()), static_cast<float>(application.GetWindow().GetHeight()));

		// Rendering
		ImGui::Render();

		if (Renderer::GetAPI() == RendererAPI::API::OpenGL)
		{
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}
		else if (Renderer::GetAPI() == RendererAPI::API::D3D11)
		{
#ifdef KBR_PLATFORM_WINDOWS
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif
		}

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backupCurrentContext = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backupCurrentContext);
		}
	}
}
