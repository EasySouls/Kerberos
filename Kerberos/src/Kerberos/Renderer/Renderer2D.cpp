#include "kbrpch.h"
#include "Renderer2D.h"

#include <glm/ext/matrix_transform.hpp>

#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"
#include "RenderCommand.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Kerberos
{
	struct Renderer2DStorage
	{
		Ref<VertexArray> VertexArray;
		Ref<Shader> Shader;
		Ref<Texture2D> Texture;
		glm::mat4 ViewProjectionMatrix;
	};

	static Renderer2DStorage* s_Data;

	void Renderer2D::Init() 
	{
		s_Data = new Renderer2DStorage();

		s_Data->VertexArray = VertexArray::Create();

		/// 3 positions, 2 texture coordinates
		constexpr float squareVertices[5 * 4] = {
			   -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
				0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
				0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			   -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		Ref<VertexBuffer> squareVB;
		squareVB.reset(VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

		squareVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
			});

		s_Data->VertexArray->AddVertexBuffer(squareVB);

		constexpr uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };

		Ref<IndexBuffer> squareIB;
		squareIB.reset(IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));

		s_Data->VertexArray->SetIndexBuffer(squareIB);

		s_Data->Shader = Shader::Create("assets/shaders/texture.glsl");

		s_Data->Texture = Texture2D::Create("assets/textures/y2k_ice_texture.png");
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->Shader)->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->Shader)->UploadUniformInt("u_Texture", 0);
	}

	void Renderer2D::Shutdown() 
	{
		delete s_Data;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera) 
	{
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->Shader)->Bind();
		const auto viewProjection = camera.GetViewProjectionMatrix();
		s_Data->ViewProjectionMatrix = viewProjection;
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->Shader)->UploadUniformMat4("u_ViewProjection", viewProjection);
	}

	void Renderer2D::EndScene() 
	{
	
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) 
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color) 
	{
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->Shader)->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->Shader)->UploadUniformFloat4("u_Color", color);

		s_Data->Texture->Bind();
		s_Data->VertexArray->Bind();

		const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->Shader)->UploadUniformMat4("u_Transform", transform);

		RenderCommand::DrawIndexed(s_Data->VertexArray);
	}
}
