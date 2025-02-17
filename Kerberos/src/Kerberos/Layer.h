#pragma once

#include "Kerberos/Events/Event.h"
#include "Kerberos/Core/Timestep.h"

namespace Kerberos
{
	class Layer
	{
	public:
		explicit Layer(std::string name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep deltaTime) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetName() const { return m_DebugName; }

	protected:
		std::string m_DebugName;
	};

}

