#pragma once

#include "Camera.h"
#include "Mesh.h"
#include "OrthographicCamera.h"
#include "Shader.h"
#include "Texture.h"
#include "Light.h"
#include "Material.h"
#include "TextureCube.h"
#include "Kerberos/Scene/EditorCamera.h"

namespace Kerberos
{
	enum class RenderPass
	{
		Shadow,
		Geometry,
		Skybox,
	};

	struct ShadowMapSettings
	{
		uint32_t Resolution = 1024;
		float NearPlane = 0.1f;
		float FarPlane = 100.0f;
		float OrthoSize = 10.0f;
		bool EnableShadows = true;
	};

	class Renderer3D
	{
    public:
		static void Init();
		static void Shutdown();

		static void BeginShadowPass(const DirectionalLight& light, const ShadowMapSettings& settings = {});

		static void BeginGeometryPass(const OrthographicCamera& camera) = delete;
		static void BeginGeometryPass(const EditorCamera& camera, const DirectionalLight* sun, const std::vector<PointLight>& pointLights, const Ref<TextureCube>& skyboxTexture);
        static void BeginGeometryPass(const Camera& camera, const glm::mat4& transform, const DirectionalLight* sun, const std::vector<PointLight>& pointLights, const Ref<TextureCube>& skyboxTexture);
		
		static void EndPass();
        static void EndScene();

		static void SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<Material>& material, const Ref<Texture2D>& texture = nullptr, float tilingFactor = 1.0f, int entityID = -1);

		static void SetGlobalAmbientLight(const glm::vec3& color, float intensity);
		static void SetShowWireframe(bool showWireframe);

        struct Statistics
        {
            uint32_t DrawCalls = 0;
            uint32_t DrawnMeshes = 0;
            uint32_t Vertices = 0;
			uint32_t Faces = 0;
        };

		static Statistics GetStatistics();
		static void ResetStatistics();

	private:
		static void SetupShadowCamera(const DirectionalLight& light, const ShadowMapSettings& settings);
		static void BindShadowMap();
	};
}

