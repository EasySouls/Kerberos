#pragma once

#include "Kerberos/Layer.h"
#include "Kerberos/Events/ApplicationEvent.h"
#include "Kerberos/Events/KeyEvent.h"
#include "Kerberos/Events/MouseEvent.h"

namespace Kerberos
{
	class KERBEROS_API ImGuiLayer final : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate() override;
		void OnEvent(Event& event) override;

	private:
		bool OnMouseButtonPressedEvent(const MouseButtonPressedEvent& event);
		bool OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& event);
		bool OnMouseMovedEvent(const MouseMovedEvent& event);
		bool OnMouseScrolledEvent(const MouseScrolledEvent& event);
		bool OnKeyPressedEvent(const KeyPressedEvent& event);
		bool OnKeyReleasedEvent(const KeyReleasedEvent& event);
		bool OnKeyTypedEvent(const KeyTypedEvent& event);
		bool OnWindowResizedEvent(const WindowResizeEvent& event);

	private:
		float m_Time = 0.0f;
	};

}

