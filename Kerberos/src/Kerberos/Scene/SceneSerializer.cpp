#include "kbrpch.h"
#include "SceneSerializer.h"

#include "Kerberos/Scene/Entity.h"
#include "Kerberos/Scene/Components.h"

#include <yaml-cpp/yaml.h>

#include <fstream>

#include "Kerberos/Assets/AssetManager.h"

namespace YAML
{
	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& vec)
		{
			Node node;
			node.push_back(vec.x);
			node.push_back(vec.y);
			node.push_back(vec.z);
			return node;
		}
		static bool decode(const Node& node, glm::vec3& vec)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;
			vec.x = node[0].as<float>();
			vec.y = node[1].as<float>();
			vec.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& vec)
		{
			Node node;
			node.push_back(vec.x);
			node.push_back(vec.y);
			node.push_back(vec.z);
			node.push_back(vec.w);
			return node;
		}
		static bool decode(const Node& node, glm::vec4& vec)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;
			vec.x = node[0].as<float>();
			vec.y = node[1].as<float>();
			vec.z = node[2].as<float>();
			vec.w = node[3].as<float>();
			return true;
		}
	};
}

namespace Kerberos
{
	static YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& vec)
	{
		out << YAML::Flow << YAML::BeginSeq << vec.x << vec.y << vec.z << YAML::EndSeq;
		return out;
	}

	static YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& vec)
	{
		out << YAML::Flow << YAML::BeginSeq << vec.x << vec.y << vec.z << vec.w << YAML::EndSeq;
		return out;
	}

	static void SerializeEntity(YAML::Emitter& out, const Entity entity)
	{
		out << YAML::BeginMap;

		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;
			const auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;

			const auto& transform = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << transform.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << transform.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << transform.Scale;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap;

			const auto& color = entity.GetComponent<SpriteRendererComponent>().Color;
			out << YAML::Key << "Color" << YAML::Value << color;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;

			const auto& cameraComponent = entity.GetComponent<CameraComponent>();
			const auto& camera = cameraComponent.Camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap;

			out << YAML::Key << "ProjectionType" << YAML::Value << static_cast<int>(camera.GetProjectionType());
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveFov();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap;

			out << YAML::Key << "IsPrimary" << YAML::Value << cameraComponent.IsPrimary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<HierarchyComponent>())
		{
			out << YAML::Key << "HierarchyComponent";
			out << YAML::BeginMap;
			const auto& hierarchy = entity.GetComponent<HierarchyComponent>();
			out << YAML::Key << "Parent" << YAML::Value << hierarchy.Parent;
			out << YAML::Key << "Children" << YAML::Value << YAML::BeginSeq;
			for (const auto& child : hierarchy.Children)
			{
				out << child;
			}
			out << YAML::EndSeq;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<NativeScriptComponent>())
		{

		}

		if (entity.HasComponent<DirectionalLightComponent>())
		{
			out << YAML::Key << "DirectionalLightComponent";
			out << YAML::BeginMap;
			const auto& directionalLight = entity.GetComponent<DirectionalLightComponent>();
			out << YAML::Key << "Color" << YAML::Value << directionalLight.Light.Color;
			out << YAML::Key << "Direction" << YAML::Value << directionalLight.Light.Direction;
			out << YAML::Key << "Intensity" << YAML::Value << directionalLight.Light.Intensity;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<PointLightComponent>())
		{
			out << YAML::Key << "PointLightComponent";
			out << YAML::BeginMap;
			const auto& pointLight = entity.GetComponent<PointLightComponent>();
			out << YAML::Key << "Color" << YAML::Value << pointLight.Light.Color;
			out << YAML::Key << "Position" << YAML::Value << pointLight.Light.Position;
			out << YAML::Key << "Intensity" << YAML::Value << pointLight.Light.Intensity;
			out << YAML::Key << "Constant" << YAML::Value << pointLight.Light.Constant;
			out << YAML::Key << "Linear" << YAML::Value << pointLight.Light.Linear;
			out << YAML::Key << "Quadratic" << YAML::Value << pointLight.Light.Quadratic;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<SpotLightComponent>())
		{
			out << YAML::Key << "SpotLightComponent";
			out << YAML::BeginMap;
			const auto& spotLight = entity.GetComponent<SpotLightComponent>();
			out << YAML::Key << "Color" << YAML::Value << spotLight.Light.Color;
			out << YAML::Key << "Position" << YAML::Value << spotLight.Light.Position;
			out << YAML::Key << "Direction" << YAML::Value << spotLight.Light.Direction;
			out << YAML::Key << "Intensity" << YAML::Value << spotLight.Light.Intensity;
			out << YAML::Key << "Constant" << YAML::Value << spotLight.Light.Constant;
			out << YAML::Key << "Linear" << YAML::Value << spotLight.Light.Linear;
			out << YAML::Key << "Quadratic" << YAML::Value << spotLight.Light.Quadratic;
			out << YAML::Key << "CutOffAngleRadians" << YAML::Value << spotLight.Light.CutOffAngleRadians;
			out << YAML::Key << "OuterCutOffAngleRadians" << YAML::Value << spotLight.Light.OuterCutOffAngleRadians;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<RigidBody3DComponent>())
		{
			out << YAML::Key << "RigidBody3DComponent";
			out << YAML::BeginMap;
			const auto& rigidBody = entity.GetComponent<RigidBody3DComponent>();
			out << YAML::Key << "Mass" << YAML::Value << rigidBody.Mass;
			out << YAML::Key << "Type" << YAML::Value << static_cast<int>(rigidBody.Type);
			out << YAML::Key << "Velocity" << YAML::Value << rigidBody.Velocity;
			out << YAML::Key << "AngularVelocity" << YAML::Value << rigidBody.AngularVelocity;
			out << YAML::Key << "UseGravity" << YAML::Value << rigidBody.UseGravity;
			out << YAML::Key << "Friction" << YAML::Value << rigidBody.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << rigidBody.Restitution;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<BoxCollider3DComponent>())
		{
			out << YAML::Key << "BoxCollider3DComponent";
			out << YAML::BeginMap;
			const auto& boxCollider = entity.GetComponent<BoxCollider3DComponent>();
			out << YAML::Key << "Size" << YAML::Value << boxCollider.Size;
			out << YAML::Key << "Offset" << YAML::Value << boxCollider.Offset;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<StaticMeshComponent>())
		{
			out << YAML::Key << "StaticMeshComponent";
			out << YAML::BeginMap;
			const auto& staticMesh = entity.GetComponent<StaticMeshComponent>();
			out << YAML::Key << "Mesh" << YAML::Value << 0; /// Placeholder for mesh index, to be replaced with actual mesh index
			if (auto& mat = staticMesh.MeshMaterial)
			{
				//out << YAML::Key << "Material" << YAML::Value << staticMesh.MeshMaterial->GetHandle();
				out << YAML::Key << "Material";
				out << YAML::BeginMap;

				out << YAML::Key << "Ambient" << YAML::Value << mat->Ambient;
				out << YAML::Key << "Diffuse" << YAML::Value << mat->Diffuse;
				out << YAML::Key << "Specular" << YAML::Value << mat->Specular;
				out << YAML::Key << "Shininess" << YAML::Value << mat->Shininess;

				out << YAML::EndMap;
			}
			//else
			//{
			//	out << YAML::Key << "Material" << YAML::Value << UUID::Invalid();
			//}

			if (staticMesh.MeshTexture)
				out << YAML::Key << "Texture" << YAML::Value << staticMesh.MeshTexture->GetHandle();
			else
				out << YAML::Key << "Texture" << YAML::Value << UUID::Invalid();
			out << YAML::EndMap;
		}

		out << YAML::EndMap;

	}

	void SceneSerializer::Serialize(const std::filesystem::path& filepath) const
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Unnamed Scene";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		for (const auto entityId : m_Scene->m_Registry.view<entt::entity>())
		{
			const Entity entity{ entityId, m_Scene.get() };
			if (!entity)
				continue;

			SerializeEntity(out, entity);
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		/// Check if there's a parent directory
		if (const std::filesystem::path parentDir = filepath.parent_path(); !parentDir.empty()) {
			std::error_code ec;
			std::filesystem::create_directories(parentDir, ec);
			if (ec) {
				KBR_CORE_ERROR("Could not create directory {0}: {1}", parentDir.string(), ec.message());
			}
		}

		if (std::ofstream fileOut(filepath); fileOut)
		{
			fileOut << out.c_str();
			fileOut.close();
		}
		else
		{
			KBR_CORE_ERROR("Could not open file {0} for writing!", filepath.string());
		}
	}

	void SceneSerializer::SerializeRuntime(const std::filesystem::path& filepath)
	{
		throw std::logic_error("Not implemented");
	}

	bool SceneSerializer::Deserialize(const std::filesystem::path& filepath) const
	{
		const std::ifstream inFile(filepath);
		std::stringstream stream;
		stream << inFile.rdbuf();

		YAML::Node data = YAML::Load(stream.str());
		if (!data["Scene"])
		{
			KBR_CORE_ERROR("Invalid Scene file {0}", filepath.string());
			return false;
		}

		auto sceneName = data["Scene"].as<std::string>();

		if (auto entities = data["Entities"])
		{
			for (const auto& entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();
				std::string tag;
				if (entity["TagComponent"])
					tag = entity["TagComponent"]["Tag"].as<std::string>();

				/// Create the entity with the given tag and id
				Entity deserializedEntity = m_Scene->CreateEntityWithUUID(tag, uuid);

				if (auto transformComponent = entity["TransformComponent"])
				{
					auto& transform = deserializedEntity.GetComponent<TransformComponent>();
					transform.Translation = transformComponent["Translation"].as<glm::vec3>();
					transform.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					transform.Scale = transformComponent["Scale"].as<glm::vec3>();
				}

				if (auto cameraComponent = entity["CameraComponent"])
				{
					auto& comp = deserializedEntity.AddComponent<CameraComponent>();
					auto cameraData = cameraComponent["Camera"];

					comp.Camera.SetProjectionType(static_cast<SceneCamera::ProjectionType>(cameraData["ProjectionType"].as<int>()));

					comp.Camera.SetPerspectiveFov(cameraData["PerspectiveFOV"].as<float>());
					comp.Camera.SetPerspectiveNearClip(cameraData["PerspectiveNear"].as<float>());
					comp.Camera.SetPerspectiveFarClip(cameraData["PerspectiveFar"].as<float>());

					comp.Camera.SetOrthographicSize(cameraData["OrthographicSize"].as<float>());
					comp.Camera.SetOrthographicNearClip(cameraData["OrthographicNear"].as<float>());
					comp.Camera.SetOrthographicFarClip(cameraData["OrthographicFar"].as<float>());

					comp.IsPrimary = cameraComponent["IsPrimary"].as<bool>();
					comp.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				}

				if (auto spriteRendererComponent = entity["SpriteRendererComponent"])
				{
					auto& spriteRenderer = deserializedEntity.AddComponent<SpriteRendererComponent>();
					spriteRenderer.Color = spriteRendererComponent["Color"].as<glm::vec4>();
				}

				if (auto hierarchyComponent = entity["HierarchyComponent"])
				{
					/// The entity must have a HierarchyComponent already when created
					auto& hierarchy = deserializedEntity.GetComponent<HierarchyComponent>();
					const uint64_t parentId = hierarchyComponent["Parent"].as<uint64_t>();
					hierarchy.Parent = UUID(parentId);
					for (const auto& child : hierarchyComponent["Children"])
					{
						const uint64_t childUUID = child.as<uint64_t>();
						hierarchy.Children.emplace_back(childUUID);
					}
				}

				if (auto nativeScriptComponent = entity["NativeScriptComponent"])
				{
					// Handle NativeScriptComponent deserialization here
					// This is a placeholder as the actual implementation depends on the scripting system used
				}

				if (auto directionalLightComponent = entity["DirectionalLightComponent"])
				{
					auto& directionalLight = deserializedEntity.AddComponent<DirectionalLightComponent>();
					directionalLight.Light.Color = directionalLightComponent["Color"].as<glm::vec3>();
					directionalLight.Light.Direction = directionalLightComponent["Direction"].as<glm::vec3>();
					directionalLight.Light.Intensity = directionalLightComponent["Intensity"].as<float>();
				}

				if (auto pointLightComponent = entity["PointLightComponent"])
				{
					auto& pointLight = deserializedEntity.AddComponent<PointLightComponent>();
					pointLight.Light.Color = pointLightComponent["Color"].as<glm::vec3>();
					pointLight.Light.Position = pointLightComponent["Position"].as<glm::vec3>();
					pointLight.Light.Intensity = pointLightComponent["Intensity"].as<float>();
					pointLight.Light.Constant = pointLightComponent["Constant"].as<float>();
					pointLight.Light.Linear = pointLightComponent["Linear"].as<float>();
					pointLight.Light.Quadratic = pointLightComponent["Quadratic"].as<float>();
				}

				if (auto spotLightComponent = entity["SpotLightComponent"])
				{
					auto& spotLight = deserializedEntity.AddComponent<SpotLightComponent>();
					spotLight.Light.Color = spotLightComponent["Color"].as<glm::vec3>();
					spotLight.Light.Position = spotLightComponent["Position"].as<glm::vec3>();
					spotLight.Light.Direction = spotLightComponent["Direction"].as<glm::vec3>();
					spotLight.Light.Intensity = spotLightComponent["Intensity"].as<float>();
					spotLight.Light.Constant = spotLightComponent["Constant"].as<float>();
					spotLight.Light.Linear = spotLightComponent["Linear"].as<float>();
					spotLight.Light.Quadratic = spotLightComponent["Quadratic"].as<float>();
					spotLight.Light.CutOffAngleRadians = spotLightComponent["CutOffAngleRadians"].as<float>();
					spotLight.Light.OuterCutOffAngleRadians = spotLightComponent["OuterCutOffAngleRadians"].as<float>();
				}

				if (auto rigidBodyComponent = entity["RigidBody3DComponent"])
				{
					auto& rigidBody = deserializedEntity.AddComponent<RigidBody3DComponent>();
					rigidBody.Mass = rigidBodyComponent["Mass"].as<float>();
					rigidBody.Type = static_cast<RigidBody3DComponent::BodyType>(rigidBodyComponent["Type"].as<int>());
					rigidBody.Velocity = rigidBodyComponent["Velocity"].as<glm::vec3>();
					rigidBody.AngularVelocity = rigidBodyComponent["AngularVelocity"].as<glm::vec3>();
					rigidBody.UseGravity = rigidBodyComponent["UseGravity"].as<bool>();
					rigidBody.Friction = rigidBodyComponent["Friction"].as<float>();
					rigidBody.Restitution = rigidBodyComponent["Restitution"].as<float>();
				}

				if (auto boxColliderComponent = entity["BoxCollider3DComponent"])
				{
					auto& boxCollider = deserializedEntity.AddComponent<BoxCollider3DComponent>();
					boxCollider.Size = boxColliderComponent["Size"].as<glm::vec3>();
					boxCollider.Offset = boxColliderComponent["Offset"].as<glm::vec3>();
				}

				if (auto staticMeshComponent = entity["StaticMeshComponent"])
				{
					auto& staticMesh = deserializedEntity.AddComponent<StaticMeshComponent>();

					/// Get the material and texture handles
					/// If they are valid, load the assets, else use default ones

					/*const auto matNode = staticMeshComponent["Material"].as<std::uint64_t>();
					const AssetHandle materialHandle = UUID(matNode);
					if (materialHandle.IsValid())
						staticMesh.MeshMaterial = AssetManager::GetAsset<Material>(materialHandle);*/

					staticMesh.MeshMaterial = CreateRef<Material>();
					if (auto matNode = staticMeshComponent["Material"])
					{
						staticMesh.MeshMaterial->Ambient = matNode["Ambient"].as<glm::vec3>();
						staticMesh.MeshMaterial->Diffuse = matNode["Diffuse"].as<glm::vec3>();
						staticMesh.MeshMaterial->Specular = matNode["Specular"].as<glm::vec3>();
						staticMesh.MeshMaterial->Shininess = matNode["Shininess"].as<float>();
					}

					const AssetHandle textureHandle = UUID(staticMeshComponent["Texture"].as<uint64_t>());
					if (textureHandle.IsValid())
						staticMesh.MeshTexture = AssetManager::GetAsset<Texture2D>(UUID(staticMeshComponent["Texture"].as<uint64_t>()));

					const auto mesh = staticMeshComponent["Mesh"].as<uint64_t>();
					if (mesh == 0)
					{
						staticMesh.StaticMesh = AssetManager::GetDefaultCubeMesh();
					}
				}
			}
		}

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::filesystem::path& filepath) const
	{
		throw std::logic_error("Not implemented");
	}
}
