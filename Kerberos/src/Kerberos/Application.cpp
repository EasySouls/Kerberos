#include "kbrpch.h"
#include "Application.h"

#include "Events/KeyEvent.h"
#include "Kerberos/Core.h"
#include "Kerberos/Renderer/Renderer.h"
#include "Kerberos/Scripting/ScriptEngine.h"

#include <GLFW/glfw3.h>

namespace Kerberos
{
	Application* Application::s_Instance = nullptr;

	/**
	 * @brief Constructs the Application singleton and initializes core subsystems.
	 *
	 * Initializes the global Application instance, sets the working directory if provided,
	 * creates the main window and its event callback, initializes the audio manager and loads a default audio resource,
	 * creates and registers the ImGui overlay, and initializes the renderer and scripting engine.
	 *
	 * @param spec Configuration used to initialize the application. Relevant fields:
	 *   - spec.Name: window title.
	 *   - spec.WorkingDirectory: if non-empty, sets the process working directory to this path; otherwise the current path is kept.
	 */
	Application::Application(const ApplicationSpecification& spec) 
	{
		KBR_PROFILE_FUNCTION();

		KBR_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Specification = spec;

		/// Set the working directory
		if (!spec.WorkingDirectory.empty())
		{
			std::filesystem::current_path(spec.WorkingDirectory);
			KBR_CORE_INFO("Working directory set to: {0}", std::filesystem::current_path().string());
		}
		else
		{
			KBR_CORE_WARN("No working directory specified, using current path: {0}", std::filesystem::current_path().string());
		}

		const WindowProps props{ spec.Name, true, 1280, 720 };
		m_Window = Window::Create(props);
		m_Window->SetEventCallback(KBR_BIND_EVENT_FN(Application::OnEvent));

		m_AudioManager = AudioManager::Create();
		m_AudioManager->Init();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		Renderer::Init();
		ScriptEngine::Init();
	}

	/**
	 * @brief Destroys the Application and cleans up global subsystems.
	 *
	 * Performs application teardown tasks, including shutting down the scripting subsystem.
	 */
	Application::~Application() 
	{
		if (m_AudioManager)
		{
			m_AudioManager->Shutdown();
			delete m_AudioManager;
			m_AudioManager = nullptr;
		}

		//Renderer::Shutdown();
		ScriptEngine::Shutdown();
	};

	void Application::Run()
	{
		while (m_Running)
		{
			// TODO: Move this to platform specific code
			const float time = static_cast<float>(glfwGetTime());
			const Timestep deltaTime = time - m_LastFrameTime;
			m_LastFrameTime = time;

			ExecuteMainThreadQueue();

			// TODO: Execute this on the render thread
			if (!m_Minimized)
			{
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(deltaTime);
			}
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}
	}

	void Application::Close() 
	{
		m_Running = false;
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(KBR_BIND_EVENT_FN(Application::OnWindowClosed));
		dispatcher.Dispatch<WindowResizeEvent>(KBR_BIND_EVENT_FN(Application::OnWindowResize));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushOverlay(overlay);
	}

	void Application::SubmitToMainThread(const std::function<void()>& function) 
	{
		std::scoped_lock lock(m_MainThreadQueueMutex);

		m_MainThreadQueue.emplace_back(function);
	}

	bool Application::OnWindowClosed(const WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(const WindowResizeEvent& e) 
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;
		Renderer::OnWindowResized(e.GetWidth(), e.GetHeight());

		return false;
	}

	void Application::ExecuteMainThreadQueue() 
	{
		std::vector<std::function<void()>> functions;
		{
			std::scoped_lock lock(m_MainThreadQueueMutex);
			functions.swap(m_MainThreadQueue);
		}
		
		for (const auto& func : functions)
			func();
	}
}