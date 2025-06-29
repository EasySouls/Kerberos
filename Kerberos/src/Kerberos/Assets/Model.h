#pragma once

#include "Kerberos/Renderer/Mesh.h"

#include <filesystem>

#include "Kerberos/Renderer/Material.h"
#include "Kerberos/Renderer/Texture.h"
#include "Kerberos/Scene/Scene.h"

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
		explicit Model(const std::filesystem::path& path, std::string name);

		/**
		 * @brief Creates the entities in the scene based on the loaded model.
		 *
		 * This has to be reworked, since for now it creates entities for each mesh in the model,
		 * and we would like one entity with a StaticMeshComponent that has all the meshes in it.
		 * @param scene The scene to which the entities will be added.
		 */
		void InitEntities(const Ref<Scene>& scene) const;

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

		std::filesystem::path m_Directory;
		std::vector<std::filesystem::path> m_LoadedTexturePaths;

		std::string m_Name;
	};
}
