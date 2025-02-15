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

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnImGuiRender() override;

		void Begin();
		void End();

	private:
		float m_Time = 0.0f;
	};

}

