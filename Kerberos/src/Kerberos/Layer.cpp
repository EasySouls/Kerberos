#include "kbrpch.h"
#include "Layer.h"

namespace Kerberos
{
	Layer::Layer(std::string name)
		: m_DebugName(std::move(name)) {}

	Layer::~Layer() = default;
}