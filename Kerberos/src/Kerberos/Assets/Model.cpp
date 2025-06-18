#include "kbrpch.h"
#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Kerberos/Core/Timer.h"

#include <ranges>
#include <algorithm>

#include "Kerberos/Scene/Components.h"
#include "Kerberos/Scene/Entity.h"

namespace Kerberos
{
	Model::Model(const std::filesystem::path& path, std::string name)
		:m_Name(std::move(name))
	{
		LoadModel(path);
	}

	void Model::InitEntities(const Ref<Scene>& scene) const
	{
		const Ref<Material> material = CreateRef<Material>();
		const Ref<Texture2D> texture = GetTextures().at(0);
		int i = 0;
		const Entity parent = scene->CreateEntity(m_Name);
		for (auto& mesh : GetMeshes())
		{
			Entity partEntity = scene->CreateEntity(m_Name + std::to_string(i));
			auto& stc = partEntity.AddComponent<StaticMeshComponent>();
			stc.StaticMesh = mesh;
			stc.MeshMaterial = material;
			stc.MeshTexture = texture;

			scene->SetParent(partEntity, parent);
			i++;
		}
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
		m_Directory = path.parent_path().string();

		KBR_CORE_TRACE("Loading model from path: {}", path.string());

		Timer timer("Model - compiling", [&](const TimerData& res)
			{
				KBR_CORE_TRACE("Model compilation took {0} ms", res.DurationMs);
			});

		ProcessNode(scene->mRootNode, scene);
	}

	void Model::ProcessNode(const aiNode* node, const aiScene* scene) 
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			KBR_CORE_TRACE("Processing mesh: {}", scene->mMeshes[node->mMeshes[i]]->mName.C_Str());
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_Meshes.push_back(ProcessMesh(mesh, scene));
		}
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			KBR_CORE_TRACE("Processing child node: {}", node->mChildren[i]->mName.C_Str());
			ProcessNode(node->mChildren[i], scene);
		}
	}

	Ref<Mesh> Model::ProcessMesh(aiMesh* mesh, const aiScene* scene) 
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<Ref<Texture2D>> textures;

		KBR_CORE_TRACE("Processing mesh with {} vertices and {} faces", mesh->mNumVertices, mesh->mNumFaces);

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex{};
			glm::vec3 vector{};

			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;

			if (mesh->HasNormals())
			{
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;
			}

			if (mesh->mTextureCoords[0])
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoord = vec;
			}
			else
				vertex.TexCoord = glm::vec2(0.0f, 0.0f);

			vertices.push_back(vertex);
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			const aiFace& face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			std::vector<Ref<Texture2D>> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			std::vector<Ref<Texture2D>> specularMaps = LoadMaterialTextures(material,
				aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		}

		m_Textures.insert(m_Textures.end(), textures.begin(), textures.end());

		return CreateRef<Mesh>(vertices, indices);
	}

	std::vector<Ref<Texture2D>> Model::LoadMaterialTextures(const aiMaterial* mat, const aiTextureType type, const std::string& typeName) 
	{
		KBR_CORE_TRACE("Loading material textures of type {} and count {}", typeName, mat->GetTextureCount(type));

		std::vector<Ref<Texture2D>> textures;
		
		for (uint32_t i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);

			const std::filesystem::path texturePath = m_Directory + "/" + str.C_Str();

			const bool textureExists = std::ranges::any_of(m_LoadedTexturePaths, [&](const std::string& texPath) {
				return texPath == texturePath.string();
				});

			if (textureExists)	
			{
				KBR_CORE_TRACE("Texture {} already loaded, skipping", texturePath.string());
				continue;
			}
			
			auto texture = Texture2D::Create(texturePath.string());
			m_LoadedTexturePaths.push_back(texturePath.string());
			textures.push_back(texture);
		}
		
		return textures;
	}
}
