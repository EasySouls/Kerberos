#include "kbrpch.h"
#include "Renderer.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace Kerberos
{
	Renderer::SceneData* Renderer::s_SceneData = new SceneData;
	Ref<UniformBuffer> Renderer::s_CameraBuffer = nullptr;

	void Renderer::Init() 
	{
		RenderCommand::SetupRendererAPI();
		RenderCommand::Init();

		s_CameraBuffer = UniformBuffer::Create(sizeof(SceneData), 0);
		s_CameraBuffer->SetData(s_SceneData, sizeof(SceneData));
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
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);

		// TODO
		/// Most of the time, this doesn't need to be updated every time
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_Transform", transform);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
}
