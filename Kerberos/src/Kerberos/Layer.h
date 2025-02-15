#pragma once

#include "Kerberos/Core.h"
#include "Kerberos/Events/Event.h"

namespace Kerberos
{
	class KERBEROS_API Layer
	{
	public:
		explicit Layer(std::string name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetName() const { return m_DebugName; }

	protected:
		std::string m_DebugName;
	};

}

