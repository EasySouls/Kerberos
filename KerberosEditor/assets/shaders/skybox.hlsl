// ==========================
// Structs
// ==========================

struct Material
{
    float3 diffuse;
    float3 specular;
    float3 ambient;
    float shininess;
};

// ==========================
// Constant buffers
// ==========================

cbuffer CameraData : register(b0)
{
    float3 u_ViewPos;
    float _pad2;
    float4x4 u_View;
    float4x4 u_Projection;
    float4x4 u_ViewProjection;
};

cbuffer PerObjectData : register(b2)
{
    int u_EntityID;
    float3 _pad3;
    float4x4 u_Model;
    Material u_Material;
};

// ==========================
// Vertex shader
// ==========================

struct VSInput
{
    float3 Position : POSITION;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float3 TexCoords : TEXCOORD0;
};

VSOutput VS_Main(VSInput input)
{
    VSOutput output;

    output.TexCoords = input.Position;

    // Remove translation from view matrix
    float4x4 viewNoTranslation = u_View;
    viewNoTranslation[3] = float4(0, 0, 0, 1);

    float4 pos = mul(float4(input.Position, 1.0f), viewNoTranslation);
    pos = mul(pos, u_Projection);

    // Push skybox to far plane
    output.Position = float4(pos.x, pos.y, pos.w, pos.w);

    return output;
}

// ==========================
// Pixel shader resources
// ==========================

TextureCube u_Skybox : register(t0);
SamplerState SkyboxSampler : register(s0);

// ==========================
// Pixel shader
// ==========================

struct PSInput
{
    float3 TexCoords : TEXCOORD0;
};

struct PSOutput
{
    float4 Color : SV_Target0;
    int EntityIDColor : SV_Target1;
};

PSOutput PS_Main(PSInput input)
{
    PSOutput output;

    float4 skyColor = u_Skybox.Sample(SkyboxSampler, input.TexCoords);

    // Material is preserved and can be used later (tinting, exposure, etc.)
    skyColor.rgb *= u_Material.diffuse;

    output.Color = skyColor;
    output.EntityIDColor = u_EntityID;

    return output;
}
