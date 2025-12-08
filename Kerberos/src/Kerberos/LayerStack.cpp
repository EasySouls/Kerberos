#include "kbrpch.h"
#include "LayerStack.h"

namespace Kerberos
{
	LayerStack::LayerStack() = default;

	LayerStack::LayerStack(LayerStack&& other) noexcept: m_Layers(std::move(other.m_Layers)),
	                                                     m_LayerInsertIndex(other.m_LayerInsertIndex)
	{
	}

	LayerStack& LayerStack::operator=(LayerStack&& other) noexcept
	{
		if (this == &other)
			return *this;
		m_Layers = std::move(other.m_Layers);
		m_LayerInsertIndex = other.m_LayerInsertIndex;
		return *this;
	}

	void LayerStack::PushLayer(LayerScope layer)
	{
		const auto& it = m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, std::move(layer));
		m_LayerInsertIndex++;
		it->get()->OnAttach();
	}

	void LayerStack::PushOverlay(LayerScope overlay)
	{
		const auto& inserted = m_Layers.emplace_back(std::move(overlay));
		inserted->OnAttach();
	}

	void LayerStack::PopLayer(const LayerScope& layer)
	{
		const auto it = std::ranges::find(m_Layers, layer);
		if (it != m_Layers.end())
		{
			//layer->OnDetach();
			m_Layers.erase(it);
			m_LayerInsertIndex--;
		}
	}
	void LayerStack::PopOverlay(const LayerScope& overlay)
	{
		const auto it = std::ranges::find(m_Layers, overlay);
		if (it != m_Layers.end())
		{
			//overlay->OnDetach();
			m_Layers.erase(it);
		}
	}
}