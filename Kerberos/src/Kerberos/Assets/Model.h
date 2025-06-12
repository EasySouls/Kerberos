#pragma once

#include "Kerberos/Renderer/Mesh.h"

#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Kerberos/Renderer/Texture.h"

namespace Kerberos
{
	class Model
	{
	public:
		explicit Model(const std::filesystem::path& path);

	private:
		void LoadModel(const std::filesystem::path& path);
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type,
		                                          std::string typeName);

	private:
		std::vector<Mesh> m_Meshes;
		std::string m_Directory;
	};
}
