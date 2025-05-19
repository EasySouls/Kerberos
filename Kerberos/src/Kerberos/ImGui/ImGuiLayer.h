#pragma once

#include "Kerberos/Layer.h"

namespace Kerberos
{
	class ImGuiLayer final : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() override;

		void OnAttach() override;
		void OnDetach() override;
		void OnEvent(Event& event) override;

		void OnImGuiRender() override;

		void BlockEvents(const bool block) { m_BlockEvents = block; }

		void Begin();
		void End();

	private:
		bool m_BlockEvents = false;
	};

}

