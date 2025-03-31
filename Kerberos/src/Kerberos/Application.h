#pragma once

#include "Window.h"
#include "ImGui/ImGuiLayer.h"
#include "Kerberos/LayerStack.h"
#include "Kerberos/Events/ApplicationEvent.h"
#include "Kerberos/Renderer/VertexArray.h"

namespace Kerberos
{
	class Application
	{
	public:

		/**
		 * Initializes the window, the renderer, and creates the ImGuiLayer.
		 */
		explicit Application(const std::string& name);
		virtual ~Application();

		void Run();
		void Close();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		static Application& Get() { return *s_Instance; }
		Window& GetWindow() const { return *m_Window; }
	private:
		bool OnWindowClosed(const WindowCloseEvent& e);
		bool OnWindowResize(const WindowResizeEvent& e);

	private:
		bool m_Running = true;
		bool m_Minimized = false;
		float m_LastFrameTime = 0;

		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		LayerStack m_LayerStack;

		static Application* s_Instance;
	};

	// To be defined in client
	Application* CreateApplication();

}

