#include "kbrpch.h"
#include "Model.h"

namespace Kerberos
{
	Model::Model(const std::filesystem::path& path) 
	{
		LoadModel(path);
	}

	void Model::LoadModel(const std::filesystem::path& path) 
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path.string(), aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			KBR_CORE_ERROR("Assimp error: {}", importer.GetErrorString());
			return;
		}
		m_Directory = path.string().substr(0, path.string().find_last_of('/'));

		ProcessNode(scene->mRootNode, scene);
	}

	void Model::ProcessNode(aiNode* node, const aiScene* scene) {}

	Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene) {}

	std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {}
}
