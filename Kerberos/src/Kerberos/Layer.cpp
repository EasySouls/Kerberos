#include "kbrpch.h"
#include "Layer.h"

#include "Application.h"

namespace Kerberos
{
	Layer::Layer(std::string name)
		: m_DebugName(std::move(name)) {}

	Layer::~Layer() = default;

	void Layer::QueueTransition(const Scope<Layer>& layerTo) const
	{
		auto& layerStack = Application::Get().GetLayerStack();
		for (auto& layerScope : layerStack.GetLayers())
		{
			if (layerScope.get() == this)
			{
				layerScope = LayerScope(layerTo.get(), LayerDeleter());
				break;
			}
		}
	}
}
