#pragma once

#include "Layer.h"
#include "Core.h"

namespace Kerberos
{
	// Custom deleter that calls OnDetach before deletion
	struct LayerDeleter
	{
		void operator()(Layer* layer) const
		{
			if (layer)
			{
				layer->OnDetach();
				delete layer;
			}
		}
	};

	using LayerScope = std::unique_ptr<Layer, LayerDeleter>;

	class LayerStack
	{
	public:
		LayerStack();
		~LayerStack() = default;

		LayerStack(const LayerStack& other) = delete;
		LayerStack(LayerStack&& other) noexcept;
		LayerStack& operator=(const LayerStack& other) = delete;
		LayerStack& operator=(LayerStack&& other) noexcept;

		void PushLayer(LayerScope layer);
		void PushOverlay(LayerScope overlay);
		void PopLayer(const LayerScope& layer);
		void PopOverlay(const LayerScope& overlay);

		std::vector<LayerScope>& GetLayers() { return m_Layers; }
		const std::vector<LayerScope>& GetLayers() const { return m_Layers; }

	private:
		std::vector<LayerScope> m_Layers;
		unsigned int m_LayerInsertIndex = 0;
	};
}

