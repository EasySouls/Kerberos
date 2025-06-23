#include "kbrpch.h"
#include "AssetRegistry.h"

namespace Kerberos
{
	//static std::mutex s_AssetRegistryMutex;

	AssetMetadata& AssetRegistry::operator[](const AssetHandle handle) 
	{
		return m_Registry[handle];
	}

	AssetMetadata& AssetRegistry::Get(const AssetHandle handle) 
	{
		KBR_CORE_ASSERT(m_Registry.contains(handle));

		return m_Registry.at(handle);
	}

	const AssetMetadata& AssetRegistry::Get(const AssetHandle handle) const 
	{
		return m_Registry.at(handle);
	}

	bool AssetRegistry::Contains(const AssetHandle handle) const 
	{
		return m_Registry.contains(handle);
	}

	size_t AssetRegistry::Remove(const AssetHandle handle) 
	{
		return m_Registry.erase(handle);
	}

	void AssetRegistry::Clear() 
	{
		m_Registry.clear();
	}
}
