#pragma once

#include "Kerberos/Core/UUID.h"
#include "Kerberos/Core.h"
#include "Kerberos/Log.h"

#include <string>

namespace Kerberos
{
	using AssetHandle = UUID;

	enum class AssetType : uint8_t
	{
		Texture2D = 0,
		TextureCube,
		Material,
		Mesh,
		Scene
	};

	class Asset
	{
	public:
		virtual ~Asset() = default;

		virtual AssetType GetType() = 0;

		UUID GetHandle() const { return m_Handle; }

	private:
		/**
		* Automatically generated UUID
		*/
		AssetHandle m_Handle;
	};

	static std::string_view AssetTypeToString(const AssetType type)
	{
		switch (type)
		{
		case AssetType::Texture2D:
			return "Texture2D";
		case AssetType::TextureCube:
			return "TextureCube";
		case AssetType::Material:
			return "Material";
		case AssetType::Mesh:
			return "Mesh";
		case AssetType::Scene:
			return "Scene";
		}

		KBR_CORE_ASSERT(false, "Unknown Asset Type!");
		return "";
	}

	static AssetType AssetTypeFromString(const std::string_view str)
	{
		if (str == "Texture2D")
			return AssetType::Texture2D;
		if (str == "TextureCube")
			return AssetType::TextureCube;
		if (str == "Material")
			return AssetType::Material;
		if (str == "Mesh")
			return AssetType::Mesh;
		if (str == "Scene")
			return AssetType::Mesh;

		KBR_CORE_ASSERT(false, "Unknown Asset Type as string!");
		return AssetType::Texture2D;
	}
}