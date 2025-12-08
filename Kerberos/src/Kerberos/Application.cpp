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
		m_LayerStack.PushOverlay(LayerScope(m_ImGuiLayer, LayerDeleter()));

		Renderer::Init();
		ScriptEngine::Init();
	}

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
				for (const LayerScope& layer : m_LayerStack.GetLayers())
					layer->OnUpdate(deltaTime);
			}
			m_ImGuiLayer->Begin();
			for (const LayerScope& layer : m_LayerStack.GetLayers())
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

		for (const auto& layer : m_LayerStack.GetLayers())
		{
			layer->OnEvent(e);
			if (e.Handled)
				break;
		}
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
