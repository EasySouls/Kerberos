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

		const DirectionalLight* SunLight;
		std::vector<PointLight> PointLights;
		static constexpr size_t MaxPointLights = 10;

		glm::vec3 GlobalAmbientColor = { 0.5f, 0.5f, 0.5f };
		float GlobalAmbientIntensity = 1.0f;
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

	void Renderer3D::BeginScene(const EditorCamera& camera, const DirectionalLight* sun,
		const std::vector<PointLight>& pointLights) 
	{
		KBR_PROFILE_FUNCTION();

		const auto& viewProjection = camera.GetViewProjectionMatrix();
		s_RendererData.ViewProjectionMatrix = viewProjection;
		s_RendererData.CameraPosition = camera.GetPosition();

		s_RendererData.ActiveShader->Bind();
		s_RendererData.ActiveShader->SetMat4("u_ViewProjection", viewProjection);
		s_RendererData.ActiveShader->SetFloat3("u_ViewPos", s_RendererData.CameraPosition);

		s_RendererData.SunLight = sun;
		s_RendererData.PointLights.clear();
		s_RendererData.PointLights.reserve(pointLights.size());
		for (const auto& pointLight : pointLights)
		{
			if (s_RendererData.PointLights.size() >= Renderer3DData::MaxPointLights)
			{
				KBR_CORE_WARN("Maximum number of point lights exceeded! Only the first {0} will be used.", Renderer3DData::MaxPointLights);
				break;
			}
			s_RendererData.PointLights.push_back(pointLight);
		}

		s_RendererData.ActiveShader->SetFloat3("u_GlobalAmbientColor", s_RendererData.GlobalAmbientColor);
		s_RendererData.ActiveShader->SetFloat("u_GlobalAmbientIntensity", s_RendererData.GlobalAmbientIntensity);
	}

	void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& transform, const DirectionalLight* sun, const std::vector<PointLight>& pointLights)
	{
		KBR_PROFILE_FUNCTION();

		const auto& viewProjection = camera.GetProjection() * glm::inverse(transform);
		s_RendererData.ViewProjectionMatrix = viewProjection;
		s_RendererData.CameraPosition = transform[3];

		s_RendererData.ActiveShader->Bind();
		s_RendererData.ActiveShader->SetMat4("u_ViewProjection", viewProjection);
		s_RendererData.ActiveShader->SetFloat3("u_ViewPos", s_RendererData.CameraPosition);

		s_RendererData.SunLight = sun;
		s_RendererData.PointLights.clear();
		s_RendererData.PointLights.reserve(pointLights.size());
		for (const auto& pointLight : pointLights)
		{
			if (s_RendererData.PointLights.size() >= Renderer3DData::MaxPointLights)
			{
				KBR_CORE_WARN("Maximum number of point lights exceeded! Only the first {0} will be used.", Renderer3DData::MaxPointLights);
				break;
			}
			s_RendererData.PointLights.push_back(pointLight);
		}

		s_RendererData.ActiveShader->SetFloat3("u_GlobalAmbientColor", s_RendererData.GlobalAmbientColor);
		s_RendererData.ActiveShader->SetFloat("u_GlobalAmbientIntensity", s_RendererData.GlobalAmbientIntensity);
	}

	void Renderer3D::EndScene() 
	{
		KBR_PROFILE_FUNCTION();
	}

	void Renderer3D::SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<Material>& material, const Ref<Texture2D>& texture, const float tilingFactor)
	{
		if (!mesh || !mesh->GetVertexArray() || mesh->GetIndexCount() == 0)
		{
			KBR_CORE_WARN("Invalid mesh or vertex array or index count!");
			return;
		}

		const Ref<Shader> shaderToUse = material->MaterialShader ? material->MaterialShader : s_RendererData.ActiveShader;
		shaderToUse->Bind();

		shaderToUse->SetMat4("u_ViewProjection", s_RendererData.ViewProjectionMatrix);
		shaderToUse->SetMat4("u_Model", transform);

		shaderToUse->SetMaterial("u_Material", material);

		if (s_RendererData.SunLight)
		{
			shaderToUse->SetInt("u_DirectionalLight.enabled", 1);
			shaderToUse->SetFloat3("u_DirectionalLight.direction", s_RendererData.SunLight->Direction);
			shaderToUse->SetFloat3("u_DirectionalLight.color", s_RendererData.SunLight->Color);
			shaderToUse->SetFloat("u_DirectionalLight.intensity", s_RendererData.SunLight->Intensity);
		}
		else
		{
			shaderToUse->SetInt("u_DirectionalLight.enabled", 0);
		}

		shaderToUse->SetInt("u_NumPointLights", static_cast<int>(s_RendererData.PointLights.size()));
		for (int i = 0; i < s_RendererData.PointLights.size(); ++i)
		{
			std::string prefix = "u_PointLights[" + std::to_string(i) + "].";
			shaderToUse->SetFloat3(prefix + "position", s_RendererData.PointLights[i].Position);
			shaderToUse->SetFloat3(prefix + "color", s_RendererData.PointLights[i].Color);
			shaderToUse->SetFloat(prefix + "intensity", s_RendererData.PointLights[i].Intensity);
			shaderToUse->SetFloat(prefix + "constant", s_RendererData.PointLights[i].Constant);
			shaderToUse->SetFloat(prefix + "linear", s_RendererData.PointLights[i].Linear);
			shaderToUse->SetFloat(prefix + "quadratic", s_RendererData.PointLights[i].Quadratic);
		}

		const Ref<Texture2D> textureToUse = texture ? texture : s_RendererData.WhiteTexture;
		constexpr int textureSlot = 0;
		textureToUse->Bind(textureSlot);
		shaderToUse->SetInt("u_Texture", textureSlot);
		shaderToUse->SetFloat("u_TilingFactor", tilingFactor);

		mesh->GetVertexArray()->Bind();
		
		RenderCommand::DrawIndexed(mesh->GetVertexArray(), mesh->GetIndexCount());

		s_Stats.DrawCalls++;
		s_Stats.DrawnMeshes++;
	}

	void Renderer3D::SetGlobalAmbientLight(const glm::vec3& color, const float intensity) 
	{
		s_RendererData.GlobalAmbientColor = color;
		s_RendererData.GlobalAmbientIntensity = intensity;
		if (s_RendererData.ActiveShader)
		{
			s_RendererData.ActiveShader->Bind();
			s_RendererData.ActiveShader->SetFloat3("u_GlobalAmbientColor", s_RendererData.GlobalAmbientColor);
			s_RendererData.ActiveShader->SetFloat("u_GlobalAmbientIntensity", s_RendererData.GlobalAmbientIntensity);
		}
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
