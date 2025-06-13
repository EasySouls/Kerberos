#pragma once

#include "Kerberos/Renderer/Mesh.h"

#include <filesystem>

#include "Kerberos/Renderer/Material.h"
#include "Kerberos/Renderer/Texture.h"

struct aiNode;
struct aiMesh;
struct aiMaterial;
struct aiScene;
enum aiTextureType;

namespace Kerberos
{
	class Model
	{
	public:
		explicit Model(const std::filesystem::path& path);

		const std::vector<Ref<Mesh>>& GetMeshes() const { return m_Meshes; }
		const std::vector<Ref<Texture2D>>& GetTextures() const { return m_Textures; }

	private:
		void LoadModel(const std::filesystem::path& path);
		void ProcessNode(const aiNode* node, const aiScene* scene);
		Ref<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Ref<Texture2D>> LoadMaterialTextures(const aiMaterial* mat, aiTextureType type,
		                                          const std::string& typeName);

	private:
		std::vector<Ref<Mesh>> m_Meshes;
		std::vector<Ref<Texture2D>> m_Textures;

		std::string m_Directory;
		std::vector<std::string> m_LoadedTexturePaths;
	};
}
