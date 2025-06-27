#include "kbrpch.h"
#include "EditorAssetManager.h"

#include "Kerberos/Assets/Importers/AssetImporter.h"
#include "Kerberos/Project/Project.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

namespace Kerberos
{
	static AssetType AssetTypeFromFileExtension(const std::filesystem::path& filepath)
	{
		const std::string extension = filepath.extension().string();
		if (extension == ".png" || extension == ".jpg" || extension == ".jpeg")
			return AssetType::Texture2D;
		else if (extension == ".fbx" || extension == ".obj")
			return AssetType::Mesh;
		else if (extension == ".kerberos")
			return AssetType::Scene;
		KBR_CORE_WARN("Unknown asset type for file: {0}", filepath.string());
		return AssetType::Texture2D;
	}

	Ref<Asset> EditorAssetManager::GetAsset(const AssetHandle handle) 
	{
		if (!IsAssetHandleValid(handle))
			return nullptr;

		Ref<Asset> asset = nullptr;
		if (IsAssetLoaded(handle))
		{
			asset = m_LoadedAssets.at(handle);
		}
		else
		{
			const AssetMetadata& metadata = GetMetadata(handle);
			asset = AssetImporter::ImportAsset(handle, metadata);
			if (!asset)
			{
				KBR_CORE_ERROR("Asset import failed!");
				return nullptr;
			}

			/// Save the loaded asset
			m_LoadedAssets[handle] = asset;
		}

		return asset;
	}

	bool EditorAssetManager::IsAssetHandleValid(const AssetHandle handle) 
	{
		if (!handle.IsValid())
			return false;

		return m_AssetRegistry.Contains(handle);
	}

	bool EditorAssetManager::IsAssetLoaded(const AssetHandle handle) 
	{
		return m_LoadedAssets.contains(handle);
	}

	void EditorAssetManager::ImportAsset(const std::filesystem::path& filepath)
	{
		AssetHandle handle;
		AssetMetadata metadata;
		metadata.Filepath = filepath;
		metadata.Type = AssetTypeFromFileExtension(filepath);

		const Ref<Asset> asset = AssetImporter::ImportAsset(handle, metadata);
		if (!asset)
		{
			KBR_CORE_ERROR("Failed to import asset: {0}", filepath.string());
			return;
		}

		handle = asset->GetHandle();
		m_AssetRegistry[handle] = metadata;
		//m_LoadedAssets[handle] = asset;
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(const AssetHandle handle) const
	{
		return m_AssetRegistry.Get(handle);
	}

	void EditorAssetManager::SerializeAssetRegistry()
	{
		const std::filesystem::path& assetDirectoryPath = Project::GetAssetDirectory();
		const std::filesystem::path assetRegistryPath = assetDirectoryPath / "AssetRegistry.kbrar";

		YAML::Emitter out;
		{
			out << YAML::BeginMap;
			out << YAML::Key << "AssetRegistry" << YAML::Value;

			for (const auto& [handle, metadata] : m_AssetRegistry)
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Handle" << YAML::Value << handle;
				out << YAML::Key << "Type" << YAML::Value << AssetTypeToString(metadata.Type);
				out << YAML::Key << "Path" << YAML::Value << metadata.Filepath.string();
				out << YAML::EndMap;
			}

			out << YAML::EndMap;
		}

		std::ofstream file(assetRegistryPath);
		if (!file.is_open())
		{
			KBR_CORE_ERROR("Could not open asset registry file for writing: {0}", assetRegistryPath.string());
			return;
		}
		file << out.c_str();
	}

	bool EditorAssetManager::DeserializeAssetRegistry()
	{
		const std::filesystem::path& assetDirectoryPath = Project::GetAssetDirectory();
		const std::filesystem::path assetRegistryPath = assetDirectoryPath / "AssetRegistry.kbrar";

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(assetRegistryPath.string());
		}
		catch (const YAML::Exception& e)
		{
			KBR_CORE_ERROR("Failed to load asset registry: {0}", e.what());
			return false;
		}

		YAML::Node registryNode;
		try
		{
			registryNode = data["AssetRegistry"];
		} 
		catch (const YAML::BadSubscript& e)
		{
			KBR_CORE_WARN("Registry was empty: {0}", e.what());
			return false;
		}

		if (!registryNode)
		{
			KBR_CORE_ERROR("Invalid asset registry file: {0}", assetRegistryPath.string());
			return false;
		}

		for (const auto& assetNode : registryNode)
		{
			if (!assetNode["Handle"] || !assetNode["Type"] || !assetNode["Path"])
			{
				KBR_CORE_ERROR("Invalid asset entry in registry: {0}", assetRegistryPath.string());
				continue;
			}
			const AssetHandle handle = AssetHandle(assetNode["Handle"].as<uint64_t>());
			const std::string typeStr = assetNode["Type"].as<std::string>();
			const std::filesystem::path filepath = assetNode["Path"].as<std::string>();

			const AssetType type = AssetTypeFromString(typeStr);
			if (type == AssetType::Texture2D)
			{
				m_AssetRegistry.Add(handle, { .Type = type, .Filepath = filepath });
			}
			else
			{
				KBR_CORE_WARN("Unsupported asset type: {0}", typeStr);
			}
		}

		KBR_CORE_INFO("Asset registry loaded from {0}", assetRegistryPath.string());

		return true;
	}
}
