#pragma once

#include "kbrpch.h"
#include "Kerberos/Events/Event.h"

namespace Kerberos
{
	struct WindowProps
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;
		bool Maximized = false;

		explicit WindowProps(std::string title = "Kerberos Engine",
			                 bool maximized = false,
		                     const uint32_t width = 1280,
		                     const uint32_t height = 720)
			: Title(std::move(title)), Width(width), Height(height), Maximized(maximized)
		{
		}
	};

	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() = default;
		
		virtual void OnUpdate() = 0;
		
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(const bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;

		static Scope<Window> Create(const WindowProps& props = WindowProps());
	};
}