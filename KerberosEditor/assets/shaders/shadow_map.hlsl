// ==========================
// Constant buffers
// ==========================

cbuffer ShadowData : register(b3)
{
    float4x4 u_LightSpaceMatrix;
    int u_EnableShadows;
    float u_ShadowBias;
    float2 _pad0;
};

struct Material
{
    float3 diffuse;
    float3 specular;
    float3 ambient;
    float shininess;
};

cbuffer PerObjectData : register(b2)
{
    int u_EntityID;
    float3 _pad1;
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
};

VSOutput VS_Main(VSInput input)
{
    VSOutput o;

    float4 worldPos = mul(float4(input.Position, 1.0f), u_Model);
    o.Position = mul(worldPos, u_LightSpaceMatrix);

    return o;
}

// ==========================
// Pixel shader
// ==========================

void PS_Main()
{
    // Depth is written automatically.
}
