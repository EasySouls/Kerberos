#pragma once

#include "Kerberos/Core.h"
#include "RenderCommand.h"
#include "OrthographicCamera.h"
#include "Shader.h"
#include "UniformBuffer.h"

namespace Kerberos
{
	class Renderer
	{
	public:
		static void Init();

		static void OnWindowResized(const uint32_t width, const uint32_t height);

		/**
		* Transfers all the uniforms to the shaders
		*/
		static void BeginScene(const OrthographicCamera& camera);

		static void EndScene();

		/// <summary>
		/// Binds the vertex array and submits it to the renderer
		/// </summary>
		/// <param name="shader">The shader to use when rendering</param>
		/// <param name="vertexArray">The vertex array to draw with</param>
		/// <param name="transform">The transformation matrix to apply to the vertex array</param>
		static void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));

		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }	

	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static SceneData* s_SceneData;

		static Ref<UniformBuffer> s_CameraBuffer;
	};
}
