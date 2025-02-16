#pragma once

#include "Window.h"
#include "ImGui/ImGuiLayer.h"
#include "Kerberos/LayerStack.h"
#include "Kerberos/Events/ApplicationEvent.h"
#include "Kerberos/Renderer/Shader.h"
#include "Kerberos/Renderer/VertexArray.h"

namespace Kerberos
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() const { return *m_Window; }
	private:
		bool OnWindowClosed(const WindowCloseEvent& e);

		bool m_Running = true;
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		LayerStack m_LayerStack;

		std::shared_ptr<Shader> m_Shader;
		std::shared_ptr<VertexArray> m_VertexArray;
		std::shared_ptr<Shader> m_BlueShader;
		std::shared_ptr<VertexArray> m_SquareVA;

		static Application* s_Instance;
	};

	// To be defined in client
	Application* CreateApplication();

}

