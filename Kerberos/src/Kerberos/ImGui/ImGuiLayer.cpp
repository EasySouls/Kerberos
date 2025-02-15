#include "kbrpch.h"
#include "ImGuiLayer.h"
#include "Platform/OpenGL/ImGuiOpenGLRenderer.h"
#include "Kerberos/Application.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>


namespace Kerberos
{
	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{}

	ImGuiLayer::~ImGuiLayer() = default;

	void ImGuiLayer::OnAttach()
	{
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		// TODO: Set up keymap

		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void ImGuiLayer::OnDetach()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnUpdate()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& application = Application::Get();
		io.DisplaySize = ImVec2(static_cast<float>(application.GetWindow().GetWidth()), static_cast<float>(application.GetWindow().GetHeight()));

		float time = static_cast<float>(glfwGetTime());
		io.DeltaTime = m_Time > 0.0f ? (time - m_Time) : (1.0f / 60.0f);
		m_Time = time;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		static bool show = true;
		ImGui::ShowDemoWindow(&show);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void ImGuiLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseButtonPressedEvent>(KBR_BIND_EVENT_FN(ImGuiLayer::OnMouseButtonPressedEvent));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(KBR_BIND_EVENT_FN(ImGuiLayer::OnMouseButtonReleasedEvent));
		dispatcher.Dispatch<MouseMovedEvent>(KBR_BIND_EVENT_FN(ImGuiLayer::OnMouseMovedEvent));
		dispatcher.Dispatch<MouseScrolledEvent>(KBR_BIND_EVENT_FN(ImGuiLayer::OnMouseScrolledEvent));
		dispatcher.Dispatch<KeyPressedEvent>(KBR_BIND_EVENT_FN(ImGuiLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<KeyReleasedEvent>(KBR_BIND_EVENT_FN(ImGuiLayer::OnKeyReleasedEvent));
		dispatcher.Dispatch<KeyTypedEvent>(KBR_BIND_EVENT_FN(ImGuiLayer::OnKeyTypedEvent));
		dispatcher.Dispatch<WindowResizeEvent>(KBR_BIND_EVENT_FN(ImGuiLayer::OnWindowResizedEvent));
	}

	bool ImGuiLayer::OnMouseButtonPressedEvent(const MouseButtonPressedEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDown[event.GetMouseButton()] = true;

		return false;
	}

	bool ImGuiLayer::OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDown[event.GetMouseButton()] = false;

		return false;
	}

	bool ImGuiLayer::OnMouseMovedEvent(const MouseMovedEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(event.GetX(), event.GetY());

		return false;
	}

	bool ImGuiLayer::OnMouseScrolledEvent(const MouseScrolledEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseWheelH += event.GetXOffset();
		io.MouseWheel += event.GetYOffset();

		return false;
	}

	bool ImGuiLayer::OnKeyPressedEvent(const KeyPressedEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		const ImGuiKey key = static_cast<ImGuiKey>(event.GetKeyCode());

		// io.AddKeyEvent(key, true);
		// TODO: Handle key release with new keymapping (imgui_impl_glfw.cpp)

		// TODO: Handle modifiers, like Ctrl, Alt, Shift, Super, etc.

		return false;
	}

	bool ImGuiLayer::OnKeyReleasedEvent(const KeyReleasedEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		const ImGuiKey key = static_cast<ImGuiKey>(event.GetKeyCode());

		// TODO: Handle key release with new keymapping (imgui_impl_glfw.cpp)
		// io.AddKeyEvent(key, false);

		return false;
	}

	bool ImGuiLayer::OnKeyTypedEvent(const KeyTypedEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();

		if (const int keycode = event.GetKeyCode(); keycode > 0 && keycode < 0x10000)
			io.AddInputCharacter(static_cast<unsigned short>(event.GetKeyCode()));

		return false;
	}

	bool ImGuiLayer::OnWindowResizedEvent(const WindowResizeEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(static_cast<float>(event.GetWidth()), static_cast<float>(event.GetHeight()));
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

		// TODO: Handle window resize
		glViewport(0, 0, event.GetWidth(), event.GetHeight());

		return false;
	}
}
