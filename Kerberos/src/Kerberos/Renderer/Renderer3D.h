#pragma once

#include "Camera.h"
#include "Mesh.h"
#include "OrthographicCamera.h"
#include "Shader.h"
#include "Texture.h"
#include "Light.h"
#include "Material.h"
#include "Kerberos/Scene/EditorCamera.h"

namespace Kerberos
{
	class Renderer3D
	{
    public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const OrthographicCamera& camera) = delete;
		static void BeginScene(const EditorCamera& camera, const DirectionalLight* sun, const std::vector<PointLight>& pointLights, bool renderSkybox = false);
        static void BeginScene(const Camera& camera, const glm::mat4& transform, const DirectionalLight* sun, const std::vector<PointLight>& pointLights);
        static void EndScene();

		static void SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<Material>& material, const Ref<Texture2D>& texture = nullptr, float tilingFactor = 1.0f, int entityID = -1);

		static void SetGlobalAmbientLight(const glm::vec3& color, float intensity);
		static void ToggleSkyboxTexture();
		static void SetShowWireframe(bool showWireframe);

        struct Statistics
        {
            uint32_t DrawCalls = 0;
            uint32_t DrawnMeshes = 0;
        };

		static Statistics GetStatistics();
		static void ResetStatistics();
	};
}

