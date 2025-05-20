#pragma once

#include <glm/glm.hpp>

#include "Camera.h"
#include "Mesh.h"
#include "OrthographicCamera.h"
#include "Shader.h"
#include "Texture.h"

namespace Kerberos
{
	class Renderer3D
	{
    public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const OrthographicCamera& camera) = delete;
        static void BeginScene(const Camera& camera, const glm::mat4& transform);
        static void EndScene();

		static void SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform = glm::mat4(1.0f), const Ref<Shader>& shader = nullptr, const Ref<Texture2D>& texture = nullptr, const glm::vec4& tintColor = {1.0f, 1.0f, 1.0f, 1.0f});

        struct Statistics
        {
            uint32_t DrawCalls = 0;
            uint32_t DrawnMeshes = 0;
        };

		static Statistics GetStatistics();
		static void ResetStatistics();
	};
}

