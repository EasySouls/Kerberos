#include "kbrpch.h"
#include "Renderer3D.h"
#include "RenderCommand.h"

namespace Kerberos
{
	struct Renderer3DData
	{
		glm::mat4 ViewProjectionMatrix;
		Ref<Shader> ActiveShader;
		Ref<Texture2D> WhiteTexture;
		glm::vec3 CameraPosition;
	};

	static Renderer3DData s_RendererData;

	static Renderer3D::Statistics s_Stats;

	void Renderer3D::Init() 
	{
		KBR_PROFILE_FUNCTION();

		s_RendererData = Renderer3DData();

		s_RendererData.ActiveShader = Shader::Create("assets/shaders/shader3d.glsl");

		s_RendererData.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_RendererData.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		ResetStatistics();
	}

	void Renderer3D::Shutdown() 
	{
		KBR_PROFILE_FUNCTION();
	}

	void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& transform) 
	{
		KBR_PROFILE_FUNCTION();

		const auto& viewProjection = camera.GetProjection() * glm::inverse(transform);
		s_RendererData.ViewProjectionMatrix = viewProjection;

		s_RendererData.ActiveShader->Bind();
		s_RendererData.ActiveShader->SetMat4("u_ViewProjection", viewProjection);
	}

	void Renderer3D::EndScene() 
	{
		KBR_PROFILE_FUNCTION();
	}

	void Renderer3D::SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<Shader>& shader, const Ref<Texture2D>& texture, const glm::vec4& tintColor, const float tilingFactor)
	{
		if (!mesh || !mesh->GetVertexArray() || mesh->GetIndexCount() == 0)
		{
			KBR_CORE_WARN("Invalid mesh or vertex array or index count!");
			return;
		}

		const Ref<Shader> shaderToUse = shader ? shader : s_RendererData.ActiveShader;
		shaderToUse->Bind();

		shaderToUse->SetMat4("u_ViewProjection", s_RendererData.ViewProjectionMatrix);
		shaderToUse->SetMat4("u_Model", transform);

		const Ref<Texture2D> textureToUse = texture ? texture : s_RendererData.WhiteTexture;
		constexpr int textureSlot = 0;
		textureToUse->Bind(textureSlot);
		shaderToUse->SetInt("u_Texture", textureSlot);
		shaderToUse->SetFloat4("u_Color", tintColor);
		shaderToUse->SetFloat("u_TilingFactor", tilingFactor);

		mesh->GetVertexArray()->Bind();
		
		RenderCommand::DrawIndexed(mesh->GetVertexArray(), mesh->GetIndexCount());

		s_Stats.DrawCalls++;
		s_Stats.DrawnMeshes++;
	}

	Renderer3D::Statistics Renderer3D::GetStatistics() 
	{
		return s_Stats;
	}

	void Renderer3D::ResetStatistics() 
	{
		s_Stats.DrawCalls = 0;
		s_Stats.DrawnMeshes = 0;
	}
}
