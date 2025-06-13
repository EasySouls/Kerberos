#pragma once

#include "Kerberos/Renderer/Mesh.h"

#include <filesystem>

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

	private:
		void LoadModel(const std::filesystem::path& path);
		void ProcessNode(const aiNode* node, const aiScene* scene);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Ref<Texture>> LoadMaterialTextures(const aiMaterial* mat, aiTextureType type,
		                                          const std::string& typeName) const;

	private:
		std::vector<Mesh> m_Meshes;
		std::string m_Directory;
	};
}
