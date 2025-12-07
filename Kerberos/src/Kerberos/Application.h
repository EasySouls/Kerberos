#pragma once

#include "Window.h"
#include "ImGui/ImGuiLayer.h"
#include "Kerberos/LayerStack.h"
#include "Kerberos/Events/ApplicationEvent.h"
#include "Kerberos/Renderer/VertexArray.h"

#include <mutex>
#include <functional>
#include <vector>

#include "Audio/AudioManager.h"

namespace Kerberos
{
	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](const int index) const
		{
			KBR_CORE_ASSERT(index < Count, "Wrong index into ApplicationCommandLineArgs");
			return Args[index];
		}
	};

	struct ApplicationSpecification
	{
		std::string Name = "Kerberos Application";
		std::string WorkingDirectory;
		ApplicationCommandLineArgs CommandLineArgs;
	};

	class Application
	{
	public:

		/**
		 * Initializes the window, the renderer, and creates the ImGuiLayer.
		 */
		explicit Application(const ApplicationSpecification& spec);
		virtual ~Application();

		Application(const Application& other) = delete;
		Application(Application&& other) noexcept = delete;
		Application& operator=(const Application& other) = delete;
		Application& operator=(Application&& other) noexcept = delete;

		void Run();
		void Close();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		void SubmitToMainThread(const std::function<void()>& function);

		static Application& Get() { return *s_Instance; }
		Window& GetWindow() const { return *m_Window; }
		ImGuiLayer* GetImGuiLayer() const { return m_ImGuiLayer; }
		const ApplicationSpecification& GetSpecification() const { return m_Specification; }
		AudioManager* GetAudioManager() const
		{
			KBR_CORE_ASSERT(m_AudioManager, "AudioManager is not initialized!");
			return m_AudioManager;
		}

	private:
		bool OnWindowClosed(const WindowCloseEvent& e);
		bool OnWindowResize(const WindowResizeEvent& e);

		void ExecuteMainThreadQueue();

	private:
		ApplicationSpecification m_Specification;
		bool m_Running = true;
		bool m_Minimized = false;
		float m_LastFrameTime = 0;

		Scope<Window> m_Window;
		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;

		std::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_MainThreadQueueMutex;

		AudioManager* m_AudioManager = nullptr;

		static Application* s_Instance;
	};

	// To be defined in client
	Application* CreateApplication(ApplicationCommandLineArgs args);

}

