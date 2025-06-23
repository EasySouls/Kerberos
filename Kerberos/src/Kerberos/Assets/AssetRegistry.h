#pragma once

#include <map>

#include "Asset.h"
#include "AssetMetadata.h"

namespace Kerberos
{
	class AssetRegistry
	{
	public:
		AssetMetadata& operator[](AssetHandle handle);
		AssetMetadata& Get(AssetHandle handle);
		const AssetMetadata& Get(AssetHandle handle) const;

		size_t Count() const { return m_Registry.size(); }
		bool Contains(AssetHandle handle) const;
		size_t Remove(AssetHandle handle);
		void Clear();

		auto begin() { return m_Registry.begin(); }
		auto end() { return m_Registry.end(); }
		auto begin() const { return m_Registry.cbegin(); }
		auto end() const { return m_Registry.cend(); }
	private:
		std::map<AssetHandle, AssetMetadata> m_Registry;
	};
}
