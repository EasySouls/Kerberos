#include "kbrpch.h"
#include "Renderer2D.h"

#include <glm/ext/matrix_transform.hpp>

#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"
#include "RenderCommand.h"

/*
* Notes:
* - We might not want to calculate the rotation and scale matrices, if we are not rotating or scaling the quad.
* - We need a Transform struct to store the position, rotation, and scale of the primitives.
*/

namespace Kerberos
{
	struct Renderer2DData
	{
		uint32_t MaxQuads = 10000;
		uint32_t MaxVertices = MaxQuads * 4;
		uint32_t MaxIndices = MaxQuads * 6;

		Ref<VertexArray> VertexArray;
		Ref<Shader> Shader;
		Ref<Texture2D> Texture;		 /// Not currently used
		Ref<Texture2D> WhiteTexture;
		glm::mat4 ViewProjectionMatrix;
	};

	static Renderer2DData* s_Data;

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;
	};

	void Renderer2D::Init() 
	{
		KBR_PROFILE_FUNCTION();

		s_Data = new Renderer2DData();

		s_Data->VertexArray = VertexArray::Create();

		const Ref<VertexBuffer> squareVB = VertexBuffer::Create(s_Data->MaxVertices * sizeof(QuadVertex));

		squareVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
			});

		s_Data->VertexArray->AddVertexBuffer(squareVB);

		constexpr uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };

		Ref<IndexBuffer> squareIB;
		squareIB.reset(IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));

		s_Data->VertexArray->SetIndexBuffer(squareIB);

		s_Data->Shader = Shader::Create("assets/shaders/shader2d.glsl");

		s_Data->WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data->WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		s_Data->Shader->Bind();
		s_Data->Shader->SetInt("u_Texture", 0);
	}

	void Renderer2D::Shutdown() 
	{
		KBR_PROFILE_FUNCTION();

		delete s_Data;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera) 
	{
		KBR_PROFILE_FUNCTION();

		const auto viewProjection = camera.GetViewProjectionMatrix();
		s_Data->ViewProjectionMatrix = viewProjection;

		s_Data->Shader->Bind();
		s_Data->Shader->SetMat4("u_ViewProjection", viewProjection);
		s_Data->Shader->SetFloat("u_TilingFactor", 1.0f);
	}

	void Renderer2D::EndScene() 
	{
		KBR_PROFILE_FUNCTION();
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const float rotation, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, rotation, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const float rotation, const glm::vec4& color)
	{
		KBR_PROFILE_FUNCTION();

		s_Data->Shader->Bind();
		s_Data->Shader->SetFloat4("u_Color", color);

		s_Data->WhiteTexture->Bind();
		s_Data->VertexArray->Bind();

		const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) 
			* glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f }) 
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		s_Data->Shader->SetMat4("u_Transform", transform);

		RenderCommand::DrawIndexed(s_Data->VertexArray);
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const float rotation, const Ref<Texture2D>& texture, const float tilingFactor, const glm::vec4& tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, rotation, texture, tilingFactor, tintColor);
	}
	
	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const float rotation, const Ref<Texture2D>& texture, const float tilingFactor, const glm::vec4& tintColor)
	{
		KBR_PROFILE_FUNCTION();

		s_Data->Shader->Bind();
		s_Data->Shader->SetFloat4("u_Color", tintColor);
		s_Data->Shader->SetFloat("u_TilingFactor", tilingFactor);

		texture->Bind();

		s_Data->VertexArray->Bind();

		const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) 
			* glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f }) 
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		s_Data->Shader->SetMat4("u_Transform", transform);

		RenderCommand::DrawIndexed(s_Data->VertexArray);
	}
}
