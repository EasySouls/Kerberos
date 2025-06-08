#include "kbrpch.h"
#include "Renderer3D.h"
#include "RenderCommand.h"
#include "TextureCube.h"
#include "UniformBuffer.h"

namespace Kerberos
{
	struct Renderer3DData
	{
		Ref<Shader> ActiveShader;
		Ref<Texture2D> WhiteTexture;

		Ref<Shader> BaseShader = nullptr;
		Ref<Shader> WireframeShader = nullptr;

		const DirectionalLight* SunLight;
		std::vector<PointLight> PointLights;
		static constexpr size_t MaxPointLights = 10;

		glm::vec3 GlobalAmbientColor = { 0.5f, 0.5f, 0.5f };
		float GlobalAmbientIntensity = 1.0f;

		bool RenderSkybox = false;
		bool RenderOceanSkybox = false;
		Ref<Shader> SkyboxShader = nullptr;
		Ref<TextureCube> StarmapSkyboxTexture = nullptr;
		Ref<TextureCube> OceanSkyboxTexture = nullptr;
		Ref<VertexArray> SkyboxVertexArray = nullptr;

		struct CameraData
		{
			glm::vec3 Position;
			glm::mat4 ViewMatrix;
			glm::mat4 ProjectionMatrix;
			glm::mat4 ViewProjectionMatrix;
		} CameraData;

		Ref<UniformBuffer> CameraUniformBuffer = nullptr;
	};

	static Renderer3DData s_RendererData;

	static Renderer3D::Statistics s_Stats;

	void Renderer3D::Init() 
	{
		KBR_PROFILE_FUNCTION();

		s_RendererData = Renderer3DData();

		s_RendererData.BaseShader = Shader::Create("assets/shaders/shader3d.glsl");
		s_RendererData.WireframeShader = Shader::Create("assets/shaders/wireframe3d.glsl");
		/// Set the default shader to the base shader
		s_RendererData.ActiveShader = s_RendererData.BaseShader;

		s_RendererData.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_RendererData.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		s_RendererData.SkyboxShader = Shader::Create("assets/shaders/skybox.glsl");
		const std::vector<std::string> skymapTextures = {
			"assets/textures/starmap_cubemap_0.png",
			"assets/textures/starmap_cubemap_1.png",
			"assets/textures/starmap_cubemap_2.png",
			"assets/textures/starmap_cubemap_3.png",
			"assets/textures/starmap_cubemap_4.png",
			"assets/textures/starmap_cubemap_5.png",

		};
		s_RendererData.StarmapSkyboxTexture = TextureCube::Create("Starmap Skybox", skymapTextures, false);
		const std::vector<std::string> oceanCubeTextures = {
			"assets/textures/skybox/right.jpg",
			"assets/textures/skybox/left.jpg",
			"assets/textures/skybox/top.jpg",
			"assets/textures/skybox/bottom.jpg",
			"assets/textures/skybox/front.jpg",
			"assets/textures/skybox/back.jpg"
		};
		s_RendererData.OceanSkyboxTexture = TextureCube::Create("Ocean Skybox", oceanCubeTextures, false);
		const std::vector<float> skyboxVertices = {
			-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
		};
		s_RendererData.SkyboxVertexArray = VertexArray::Create();
		const Ref<VertexBuffer> skyboxVertexBuffer = VertexBuffer::Create(skyboxVertices.data(), static_cast<uint32_t>(skyboxVertices.size()) * sizeof(float));
		skyboxVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" }
			});
		s_RendererData.SkyboxVertexArray->AddVertexBuffer(skyboxVertexBuffer);

		s_RendererData.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DData::CameraData), 0);

		ResetStatistics();
	}

	void Renderer3D::Shutdown() 
	{
		KBR_PROFILE_FUNCTION();
	}

	void Renderer3D::BeginScene(const EditorCamera& camera, const DirectionalLight* sun,
		const std::vector<PointLight>& pointLights, const bool renderSkybox) 
	{
		KBR_PROFILE_FUNCTION();

		const auto& viewProjection = camera.GetViewProjectionMatrix();
		s_RendererData.CameraData.ViewMatrix = camera.GetViewMatrix();
		s_RendererData.CameraData.ProjectionMatrix = camera.GetProjection();
		s_RendererData.CameraData.ViewProjectionMatrix = viewProjection;
		s_RendererData.CameraData.Position = camera.GetPosition();

		s_RendererData.ActiveShader->Bind();
		s_RendererData.CameraUniformBuffer->SetData(&s_RendererData.CameraData, sizeof(Renderer3DData::CameraData), 0);

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

		s_RendererData.RenderSkybox = renderSkybox;
	}

	void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& transform, const DirectionalLight* sun, const std::vector<PointLight>& pointLights)
	{
		KBR_PROFILE_FUNCTION();

		const auto& viewProjection = camera.GetProjection() * glm::inverse(transform);
		s_RendererData.CameraData.ViewMatrix = glm::inverse(transform);
		s_RendererData.CameraData.ProjectionMatrix = camera.GetProjection();
		s_RendererData.CameraData.ViewProjectionMatrix = viewProjection;
		s_RendererData.CameraData.Position = transform[3];

		s_RendererData.ActiveShader->Bind();
		s_RendererData.CameraUniformBuffer->SetData(&s_RendererData.CameraData, sizeof(Renderer3DData::CameraData), 0);

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

		/// Render the skybox last if enabled
		if (!s_RendererData.RenderSkybox)
			return;

		RenderCommand::SetDepthFunc(DepthFunc::LessEqual);
		s_RendererData.SkyboxShader->Bind();

		/// Remove translation from view matrix
		const glm::mat4 skyboxView = glm::mat4(glm::mat3(s_RendererData.CameraData.ViewMatrix)); 

		s_RendererData.SkyboxShader->SetMat4("u_View", skyboxView);
		s_RendererData.SkyboxShader->SetMat4("u_Projection", s_RendererData.CameraData.ProjectionMatrix);

		/// Set the hovered entity's id to an invalid value
		s_RendererData.SkyboxShader->SetInt("u_EntityID", -1);

		s_RendererData.SkyboxVertexArray->Bind();
		if (s_RendererData.RenderOceanSkybox)
			s_RendererData.OceanSkyboxTexture->Bind(0);
		else
			s_RendererData.StarmapSkyboxTexture->Bind(0);

		/// Always 36 indices for the skybox
		RenderCommand::DrawArray(s_RendererData.SkyboxVertexArray, 36);

		/// Reset depth function to default
		RenderCommand::SetDepthFunc(DepthFunc::Less); 

		s_Stats.DrawCalls++;
		s_Stats.DrawnMeshes++;
	}

	void Renderer3D::SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<Material>& material, const Ref<Texture2D>& texture, const float tilingFactor, const int entityID)
	{
		if (!mesh || !mesh->GetVertexArray() || mesh->GetIndexCount() == 0)
		{
			KBR_CORE_WARN("Invalid mesh or vertex array or index count!");
			return;
		}

		const Ref<Shader> shaderToUse = material->MaterialShader ? material->MaterialShader : s_RendererData.ActiveShader;
		shaderToUse->Bind();

		shaderToUse->SetMat4("u_ViewProjection", s_RendererData.CameraData.ViewProjectionMatrix);
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
		shaderToUse->SetInt("u_EntityID", entityID);

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

	void Renderer3D::ToggleSkyboxTexture() 
	{
		s_RendererData.RenderOceanSkybox = !s_RendererData.RenderOceanSkybox;
	}

	void Renderer3D::SetShowWireframe(const bool showWireframe) 
	{
		s_RendererData.ActiveShader = showWireframe ? s_RendererData.WireframeShader : s_RendererData.BaseShader;
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
