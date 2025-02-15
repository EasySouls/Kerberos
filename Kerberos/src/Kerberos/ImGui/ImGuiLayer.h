#pragma once

#include "Kerberos/Layer.h"

namespace Kerberos
{
	class KERBEROS_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate() override;
		void OnEvent(Event& event) override;

		void Begin();
		void End();
	private:
		float m_Time = 0.0f;
	};

}

