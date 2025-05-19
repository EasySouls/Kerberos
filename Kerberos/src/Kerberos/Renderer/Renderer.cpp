#include "kbrpch.h"
#include "Renderer.h"

#include "Renderer2D.h"

namespace Kerberos
{
	Renderer::SceneData* Renderer::s_SceneData = new SceneData;
	Ref<UniformBuffer> Renderer::s_CameraBuffer = nullptr;

	void Renderer::Init()
	{
		RenderCommand::SetupRendererAPI();
		RenderCommand::Init();
		Renderer2D::Init();
	}

	void Renderer::OnWindowResized(const uint32_t width, const uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(const OrthographicCamera& camera)
	{
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();

		s_CameraBuffer->SetData(&s_SceneData->ViewProjectionMatrix, sizeof(SceneData));
	}

	void Renderer::EndScene()
	{

	}

	void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		shader->Bind();
		shader->SetMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);

		shader->SetMat4("u_Transform", transform);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
}
