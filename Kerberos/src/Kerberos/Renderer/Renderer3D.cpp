#include "kbrpch.h"
#include "Renderer3D.h"

#include "Framebuffer.h"
#include "RenderCommand.h"
#include "TextureCube.h"
#include "UniformBuffer.h"
#include "Kerberos/Assets/AssetManager.h"

static constexpr int MAX_POINT_LIGHTS = 10;

namespace Kerberos
{
	struct MaterialUbo
	{
		alignas(16) glm::vec3 Diffuse = glm::vec3{ 1.0f };
		alignas(16) glm::vec3 Specular = glm::vec3{ 0.1f };
		alignas(16) glm::vec3 Ambient = glm::vec3{ 0.1f };
		alignas(4) float Shininess = 10.f;
	};

	struct Renderer3DData
	{
		Ref<Shader> ActiveShader;
		Ref<Texture2D> WhiteTexture;

		Ref<Shader> GeometryShader = nullptr;
		Ref<Shader> WireframeShader = nullptr;
		Ref<Shader> ShadowMapShader = nullptr;

		Ref<Shader>			TextShader = nullptr;
		Ref<VertexArray>	TextVertexArray = nullptr;
		Ref<VertexBuffer>	TextVertexBuffer = nullptr;
		Ref<IndexBuffer>	TextIndexBuffer = nullptr;

		const DirectionalLight* pSunLight = nullptr;

		Ref<Shader>			SkyboxShader = nullptr;
		Ref<TextureCube>	SkyboxTexture = nullptr;
		Ref<VertexArray>	SkyboxVertexArray = nullptr;

		Ref<Framebuffer> ShadowMapFramebuffer = nullptr;

		RenderPass CurrentPass = RenderPass::Geometry;

		struct ShadowDataUbo
		{
			alignas(16) glm::mat4	LightSpaceMatrix;
			alignas(4) int			EnableShadows = 1;
			alignas(4) float		ShadowBias = 0.005f;
		} ShadowData;

		Ref<UniformBuffer> ShadowUniformBuffer = nullptr;

		struct CameraDataUbo
		{
			alignas(16) glm::vec3	Position;
			alignas(16) glm::mat4	ViewMatrix;
			glm::mat4				ProjectionMatrix;
			glm::mat4				ViewProjectionMatrix;
		} CameraData;

		Ref<UniformBuffer> CameraUniformBuffer = nullptr;

		struct LightsDataUbo
		{
			glm::vec3			GlobalAmbientColor = { 1.0f, 1.0f, 1.0f };
			alignas(4) float	GlobalAmbientIntensity = 2.0f;

			alignas(4) int NrOfPointLights = 0;
			DirectionalLight SunLight;
			std::array<PointLight, MAX_POINT_LIGHTS> PointLights;
		} LightsData;

		Ref<UniformBuffer> LightsUniformBuffer = nullptr;

		struct PerObjectDataUbo
		{
			int EntityID = -1;
			alignas(16) glm::mat4 ModelMatrix;
			alignas(16) MaterialUbo Material;
		} PerObjectData;

		Ref<UniformBuffer> PerObjectUniformBuffer = nullptr;

		/// The currently active texture, used for binding textures
		/// This is used to avoid binding the same texture multiple times
		Ref<Texture2D> ActiveTexture = nullptr;

		constexpr static uint32_t MaterialTextureSlot = 0;
		constexpr static uint32_t ShadowMapTextureSlot = 1;
		constexpr static uint32_t FontAtlasTextureSlot = 2;
	};

	static Renderer3DData s_RendererData;

	static Renderer3D::Statistics s_Stats;


	void Renderer3D::Init() 
	{
		KBR_PROFILE_FUNCTION();

		KBR_CORE_INFO("Renderer3D initialized with max point lights: {0}", MAX_POINT_LIGHTS);
		KBR_CORE_INFO("Size of CameraData: {0} bytes", sizeof(Renderer3DData::CameraData));
		KBR_CORE_INFO("Size of LightsData: {0} bytes", sizeof(Renderer3DData::LightsData));
		KBR_CORE_INFO("Size of PerObjectData: {0} bytes", sizeof(Renderer3DData::PerObjectData));

		s_RendererData = Renderer3DData();

		s_RendererData.GeometryShader = Shader::Create("assets/shaders/shader3d.glsl");
		s_RendererData.WireframeShader = Shader::Create("assets/shaders/wireframe3d.glsl");
		s_RendererData.ShadowMapShader = Shader::Create("assets/shaders/shadow_map.glsl");
		s_RendererData.TextShader = Shader::Create("assets/shaders/text.glsl");

		s_RendererData.GeometryShader->SetDebugName("Geometry");
		s_RendererData.WireframeShader->SetDebugName("Wireframe");
		s_RendererData.ShadowMapShader->SetDebugName("Shadow Map");
		s_RendererData.TextShader->SetDebugName("Text");

		/// Set the default shader to the base shader
		s_RendererData.ActiveShader = s_RendererData.GeometryShader;

		s_RendererData.WhiteTexture = AssetManager::GetDefaultTexture2D();
		s_RendererData.ActiveTexture = s_RendererData.WhiteTexture;

		s_RendererData.SkyboxShader = Shader::Create("assets/shaders/skybox.glsl");
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
		s_RendererData.SkyboxShader->SetDebugName("Skybox");

		s_RendererData.TextVertexArray = VertexArray::Create();
		s_RendererData.TextVertexArray->SetDebugName("Text Vertex Array");
		const std::array<uint32_t, 6> textIndices = { 0, 1, 2, 2, 3, 0 };
		s_RendererData.TextIndexBuffer = IndexBuffer::Create(textIndices.data(), static_cast<uint32_t>(textIndices.size()));
		s_RendererData.TextIndexBuffer->SetDebugName("Text Index Buffer");
		s_RendererData.TextVertexArray->SetIndexBuffer(s_RendererData.TextIndexBuffer);
		constexpr uint32_t textVertexBufferSize = sizeof(TextVertex) * 4;
		s_RendererData.TextVertexBuffer = VertexBuffer::Create(textVertexBufferSize);
		s_RendererData.TextVertexBuffer->SetDebugName("Text Vertex Buffer");
		s_RendererData.TextVertexBuffer->SetLayout(TextVertex::GetLayout());
		s_RendererData.TextVertexArray->AddVertexBuffer(s_RendererData.TextVertexBuffer);

		s_RendererData.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DData::CameraData), 0);
		s_RendererData.CameraUniformBuffer->SetDebugName("Camera Uniform Buffer");

		s_RendererData.LightsUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DData::LightsData), 1);
		s_RendererData.LightsUniformBuffer->SetDebugName("Lights Uniform Buffer");

		s_RendererData.PerObjectUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DData::PerObjectData), 2);
		s_RendererData.PerObjectUniformBuffer->SetDebugName("PerObject Uniform Buffer");

		s_RendererData.ShadowUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DData::ShadowDataUbo), 3);
		s_RendererData.ShadowUniformBuffer->SetDebugName("Shadow Uniform Buffer");

		ResetStatistics();
	}

	void Renderer3D::Shutdown() 
	{
		KBR_PROFILE_FUNCTION();
	}

	void Renderer3D::BeginShadowPass(const DirectionalLight& light, const ShadowMapSettings& settings, const Ref<Framebuffer>& shadowMapFramebuffer) 
	{
		KBR_PROFILE_FUNCTION();

		s_RendererData.CurrentPass = RenderPass::Shadow;

		s_RendererData.ShadowMapFramebuffer = shadowMapFramebuffer;
		shadowMapFramebuffer->Bind();

		RenderCommand::ClearDepth();
		//shadowMapFramebuffer->ClearDepthAttachment(0);

		SetupShadowCamera(light, settings);

		s_RendererData.ActiveShader = s_RendererData.ShadowMapShader;
		s_RendererData.ActiveShader->Bind();
	}

	void Renderer3D::EndPass() 
	{
		if (s_RendererData.CurrentPass == RenderPass::Shadow)
		{
			s_RendererData.ShadowMapFramebuffer->Unbind();
		}
	}

	void Renderer3D::BeginGeometryPass(const EditorCamera& camera, const DirectionalLight* sun,
	                                   const std::vector<PointLight>& pointLights, const Ref<TextureCube>& skyboxTexture) 
	{
		KBR_PROFILE_FUNCTION();

		s_RendererData.CurrentPass = RenderPass::Geometry;

		const auto& viewProjection = camera.GetViewProjectionMatrix();
		s_RendererData.CameraData.ViewMatrix = camera.GetViewMatrix();
		s_RendererData.CameraData.ProjectionMatrix = camera.GetProjection();
		s_RendererData.CameraData.ViewProjectionMatrix = viewProjection;
		s_RendererData.CameraData.Position = camera.GetPosition();

		s_RendererData.ActiveShader = s_RendererData.GeometryShader;
		s_RendererData.ActiveShader->Bind();

		BindShadowMap();

		s_RendererData.CameraUniformBuffer->SetData(&s_RendererData.CameraData, sizeof(Renderer3DData::CameraData), 0);

		s_RendererData.pSunLight = sun;
		
		if (!sun)
			s_RendererData.LightsData.SunLight = DirectionalLight{.IsEnabled = false};
		else
			s_RendererData.LightsData.SunLight = *sun;

		s_RendererData.LightsData.NrOfPointLights = static_cast<int>(pointLights.size());
		
		for (size_t i = 0; i < pointLights.size(); ++i)
		{
			if (i >= MAX_POINT_LIGHTS)
			{
				KBR_CORE_WARN("Maximum number of point lights exceeded! Only the first {0} will be used.", MAX_POINT_LIGHTS);
				break;
			}
			s_RendererData.LightsData.PointLights[i] = pointLights[i];
		}

		s_RendererData.LightsUniformBuffer->SetData(&s_RendererData.LightsData, sizeof(Renderer3DData::LightsData), 0);

		s_RendererData.SkyboxTexture = skyboxTexture;
	}

	void Renderer3D::BeginGeometryPass(const Camera& camera, const glm::mat4& transform, const DirectionalLight* sun, const std::vector<PointLight>& pointLights, const Ref<TextureCube>& skyboxTexture)
	{
		KBR_PROFILE_FUNCTION();

		s_RendererData.CurrentPass = RenderPass::Geometry;

		const glm::mat4& viewProjection = camera.GetProjection() * glm::inverse(transform);
		s_RendererData.CameraData.ViewMatrix = glm::inverse(transform);
		s_RendererData.CameraData.ProjectionMatrix = camera.GetProjection();
		s_RendererData.CameraData.ViewProjectionMatrix = viewProjection;
		s_RendererData.CameraData.Position = transform[3];

		s_RendererData.ActiveShader = s_RendererData.GeometryShader;
		s_RendererData.ActiveShader->Bind();

		BindShadowMap();

		s_RendererData.CameraUniformBuffer->SetData(&s_RendererData.CameraData, sizeof(Renderer3DData::CameraData), 0);

		s_RendererData.pSunLight = sun;
		
		if (!sun)
			s_RendererData.LightsData.SunLight = DirectionalLight{ .IsEnabled = false };
		else
			s_RendererData.LightsData.SunLight = *sun;

		s_RendererData.LightsData.NrOfPointLights = static_cast<int>(pointLights.size());

		for (size_t i = 0; i < pointLights.size(); ++i)
		{
			if (i >= MAX_POINT_LIGHTS)
			{
				KBR_CORE_WARN("Maximum number of point lights exceeded! Only the first {0} will be used.", MAX_POINT_LIGHTS);
				break;
			}
			s_RendererData.LightsData.PointLights[i] = pointLights[i];
		}

		s_RendererData.LightsUniformBuffer->SetData(&s_RendererData.LightsData, sizeof(Renderer3DData::LightsData), 0);

		s_RendererData.SkyboxTexture = skyboxTexture;
	}

	void Renderer3D::EndScene() 
	{
		KBR_PROFILE_FUNCTION();

		/// Render the skybox last if enabled
		if (s_RendererData.SkyboxTexture == nullptr)
			return;

		RenderCommand::SetDepthFunc(DepthFunc::LessEqual);
		s_RendererData.SkyboxShader->Bind();

		/// Remove translation from view matrix
		const glm::mat4 skyboxView = glm::mat4(glm::mat3(s_RendererData.CameraData.ViewMatrix)); 

		/// Changing only the view matrix is enough
		s_RendererData.CameraData.ViewMatrix = skyboxView;
		constexpr int viewMatrixOffset = offsetof(Renderer3DData::CameraDataUbo, ViewMatrix);
		s_RendererData.CameraUniformBuffer->SetData(&s_RendererData.CameraData.ViewMatrix, sizeof(Renderer3DData::CameraDataUbo::ViewMatrix), viewMatrixOffset);

		/// Set the hovered entity's id to an invalid value
		s_RendererData.SkyboxShader->SetInt("u_EntityID", -1);

		s_RendererData.SkyboxVertexArray->Bind();
		s_RendererData.SkyboxTexture->Bind(0);

		/// Always 36 indices for the skybox
		RenderCommand::DrawArray(s_RendererData.SkyboxVertexArray, 36);

		/// Reset depth function to default
		RenderCommand::SetDepthFunc(DepthFunc::Less); 

		s_Stats.Vertices += 36;
		s_Stats.Faces += 12;
		s_Stats.DrawCalls++;
		s_Stats.DrawnMeshes++;
	}

	void Renderer3D::SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<Material>& material, const Ref<Texture2D>& texture, const float tilingFactor, const int entityID, const bool castShadows)
	{
		if (!mesh || !mesh->GetVertexArray() || mesh->GetIndexCount() == 0)
		{
			KBR_CORE_WARN("Invalid mesh or vertex array or index count!");
			return;
		}

		if (s_RendererData.CurrentPass == RenderPass::Shadow && !castShadows)
		{
			/// Skip rendering this mesh in shadow pass if it doesn't cast shadows
			return;
		}

		//const Ref<Shader> shaderToUse = material->MaterialShader ? material->MaterialShader : s_RendererData.ActiveShader;
		const Ref<Shader> shaderToUse = s_RendererData.ActiveShader;
		shaderToUse->Bind();

		s_RendererData.PerObjectData.ModelMatrix = transform;
		s_RendererData.PerObjectData.EntityID = entityID;
		s_RendererData.PerObjectData.Material = { .Diffuse = material->Diffuse,
			.Specular = material->Specular, .Ambient = material->Ambient, .Shininess = material->Shininess };

		s_RendererData.PerObjectUniformBuffer->SetData(&s_RendererData.PerObjectData, sizeof(Renderer3DData::PerObjectData), 0);

		const Ref<Texture2D> textureToUse = texture ? texture : s_RendererData.WhiteTexture;
		constexpr int textureSlot = Renderer3DData::MaterialTextureSlot;
		if (s_RendererData.ActiveTexture != textureToUse)
		{
			s_RendererData.ActiveTexture = textureToUse;
			textureToUse->Bind(textureSlot);
		}
		shaderToUse->SetInt("u_Texture", textureSlot);

		mesh->GetVertexArray()->Bind();
		
		RenderCommand::DrawIndexed(mesh->GetVertexArray(), mesh->GetIndexCount());

		s_Stats.DrawCalls++;
		s_Stats.DrawnMeshes++;
		s_Stats.Vertices += mesh->GetVertexCount();
		s_Stats.Faces += mesh->GetIndexCount() / 3;
	}

	void Renderer3D::SubmitText(const std::string& text, const Ref<Font>& font, const glm::mat4& transform,
		const glm::vec4& color, const float fontSize, int entityID)
	{
		const Ref<Texture2D> fontAtlas = font->GetAtlasTexture();
		const FontMetrics& metrics = font->GetMetrics();

		constexpr int fontAtlasTextureSlot = Renderer3DData::FontAtlasTextureSlot;
		s_RendererData.TextShader->Bind();
		s_RendererData.TextShader->SetInt("u_FontAtlas", fontAtlasTextureSlot);
		fontAtlas->Bind(fontAtlasTextureSlot);

		float textScale = fontSize * 0.1f;
		glm::mat4 scaledTransform = transform * glm::scale(glm::mat4(1.0f), glm::vec3(textScale));

		s_RendererData.PerObjectData.ModelMatrix = scaledTransform;
		s_RendererData.PerObjectData.EntityID = entityID; // Currently this is not used, we submit the entity ID per vertex

		/*constexpr int modelMatrixOffset = offsetof(Renderer3DData::PerObjectDataUbo, ModelMatrix);
		s_RendererData.PerObjectUniformBuffer->SetData(&s_RendererData.PerObjectData, sizeof(Renderer3DData::PerObjectData.ModelMatrix), modelMatrixOffset);*/
		s_RendererData.PerObjectUniformBuffer->SetData(&s_RendererData.PerObjectData, sizeof(Renderer3DData::PerObjectData), 0);

		double x = 0.0;
		const double fsScale = 1.0 / (metrics.Ascender - metrics.Descender);
		double y = 0.0;

		for (size_t i = 0; i < text.size(); ++i)
		{
			char character = text[i];
			if (character == '\r')
				continue;

			if (character == '\n')
			{
				float lineHeightOffset = 0.0f;
				x = 0;
				y -= fsScale * metrics.LineHeight + lineHeightOffset;
				continue;
			}

			const bool hasGlyph = font->HasCharacter(character);
			if (!hasGlyph) 
			{
				constexpr char placeholderChar = '?';
				if (!font->HasCharacter(placeholderChar))
				{
					KBR_CORE_WARN("Font does not contain character '{}' and no placeholder character '{}' found!", character, placeholderChar);
					return;
				}
				character = placeholderChar;
			}

			if (character == '\t') {
				character = ' ';
			}

			double al, ab, ar, at;
			font->GetQuadAtlasBounds(character, al, ab, ar, at);
			glm::vec2 texCoordMin(static_cast<float>(al), static_cast<float>(ab));
			glm::vec2 texCoordMax(static_cast<float>(ar), static_cast<float>(at));

			double pl, pb, pr, pt;
			font->GetQuadPlaneBounds(character, pl, pb, pr, pt);
			glm::vec2 quadMin(static_cast<float>(pl), static_cast<float>(pb));
			glm::vec2 quadMax(static_cast<float>(pr), static_cast<float>(pt));

			quadMin *= fsScale;
			quadMax *= fsScale;
			quadMin += glm::vec2(x, y);
			quadMax += glm::vec2(x, y);

			float texelWidth = 1.0f / static_cast<float>(fontAtlas->GetWidth());
			float texelHeight = 1.0f / static_cast<float>(fontAtlas->GetHeight());
			texCoordMin *= glm::vec2(texelWidth, texelHeight);
			texCoordMax *= glm::vec2(texelWidth, texelHeight);

			std::array<TextVertex, 4> vertices;
			vertices[0].Position = glm::vec3(quadMin, 0.0f);
			vertices[0].Color = color;
			vertices[0].TexCoord = texCoordMin;
			vertices[0].EntityID = entityID;

			vertices[1].Position = glm::vec3(quadMin.x, quadMax.y, 0.0f);
			vertices[1].Color = color;
			vertices[1].TexCoord = { texCoordMin.x, texCoordMax.y };
			vertices[1].EntityID = entityID;

			vertices[2].Position = glm::vec3(quadMax, 0.0f);
			vertices[2].Color = color;
			vertices[2].TexCoord = texCoordMax;
			vertices[2].EntityID = entityID;

			vertices[3].Position = glm::vec3(quadMax.x, quadMin.y, 0.0f);
			vertices[3].Color = color;
			vertices[3].TexCoord = { texCoordMax.x, texCoordMin.y };
			vertices[3].EntityID = entityID;

			constexpr uint32_t vbSize = static_cast<uint32_t>(vertices.size()) * sizeof(TextVertex);
			s_RendererData.TextVertexBuffer->SetData(vertices.data(), vbSize);

			RenderCommand::DrawIndexed(s_RendererData.TextVertexArray, 6);

			s_Stats.Vertices += 4;
			s_Stats.Faces += 2;
			s_Stats.DrawCalls++;

			if (i < text.size() - 1)
			{
				double advance = font->GetAdvance(character);
				char nextCharacter = text[i + 1];
				font->GetNextAdvance(advance, character, nextCharacter);

				float kerningOffset = 0.0f;
				x += fsScale * advance + kerningOffset;
			}
		}
	}

	void Renderer3D::SetGlobalAmbientLight(const glm::vec3& color, const float intensity) 
	{
		s_RendererData.LightsData.GlobalAmbientColor = color;
		s_RendererData.LightsData.GlobalAmbientIntensity = intensity;
		if (s_RendererData.ActiveShader)
		{
			s_RendererData.ActiveShader->Bind();
			s_RendererData.ActiveShader->SetFloat3("u_GlobalAmbientColor", s_RendererData.LightsData.GlobalAmbientColor);
			s_RendererData.ActiveShader->SetFloat("u_GlobalAmbientIntensity", s_RendererData.LightsData.GlobalAmbientIntensity);
		}
	}

	void Renderer3D::SetShowWireframe(const bool showWireframe) 
	{
		s_RendererData.ActiveShader = showWireframe ? s_RendererData.WireframeShader : s_RendererData.GeometryShader;
	}

	Renderer3D::Statistics Renderer3D::GetStatistics() 
	{
		return s_Stats;
	}

	void Renderer3D::ResetStatistics() 
	{
		s_Stats.DrawCalls = 0;
		s_Stats.DrawnMeshes = 0;
		s_Stats.Faces = 0;
		s_Stats.Vertices = 0;
	}

	void Renderer3D::SetupShadowCamera(const DirectionalLight& light, const ShadowMapSettings& settings)
	{
		/// Create orthographic projection for directional light
		const float orthoLeft = -settings.OrthoSize;
		const float orthoRight = settings.OrthoSize;
		const float orthoBottom = -settings.OrthoSize;
		const float orthoTop = settings.OrthoSize;
		const float orthoNear = settings.NearPlane;
		const float orthoFar = settings.FarPlane;

		const glm::mat4 lightProjection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, orthoNear, orthoFar);

		/// Calculate light view matrix
		constexpr glm::vec3 sceneCenter = glm::vec3(0.0f, 0.0f, 0.0f);
		constexpr float lightDistance = 20.0f;

		const glm::vec3 lightPos = sceneCenter - glm::normalize(light.Direction) * lightDistance; /// Position light away from origin
		constexpr glm::vec3 lightTarget = sceneCenter; /// Look at origin
		constexpr glm::vec3 lightUp = glm::vec3(0.0f, 1.0f, 0.0f);

		const glm::mat4 lightView = glm::lookAt(lightPos, lightTarget, lightUp);

		/// Calculate light space matrix
		s_RendererData.ShadowData.LightSpaceMatrix = lightProjection * lightView;
		s_RendererData.ShadowData.EnableShadows = settings.EnableShadows ? 1 : 0;
		s_RendererData.ShadowUniformBuffer->SetData(&s_RendererData.ShadowData, sizeof(Renderer3DData::ShadowDataUbo), 0);
	}
	void Renderer3D::BindShadowMap()
	{
		/*auto shadowMapTexture = s_RendererData.ShadowMapFramebuffer->GetDepthAttachmentRendererID();*/
		s_RendererData.ShadowMapFramebuffer->BindDepthTexture(1);
		constexpr int shadowMapTextureSlot = Renderer3DData::ShadowMapTextureSlot;
		s_RendererData.ActiveShader->SetInt("u_ShadowMap", shadowMapTextureSlot);
	}
}
