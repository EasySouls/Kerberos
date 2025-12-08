#pragma once

#include "Kerberos/Events/Event.h"
#include "Kerberos/Core/Timestep.h"
#include "Kerberos/Core.h"

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

		template<typename TLayer, typename... Args>
			requires std::derived_from<TLayer, Layer>
		void TransitionTo(Args&&... args)
		{
			QueueTransition(std::move(CreateRef<TLayer>(std::forward<Args>(args)...)));
		}

		inline const std::string& GetName() const { return m_DebugName; }

	private:
		void QueueTransition(const Scope<Layer>& layerTo) const;

	protected:
		std::string m_DebugName;
	};

}

