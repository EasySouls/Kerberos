// ==========================
// Constant buffers
// ==========================

cbuffer Camera : register(b0)
{
    float3 u_CameraPosition;
    float _pad0;
    float4x4 u_ViewMatrix;
    float4x4 u_ProjectionMatrix;
    float4x4 u_ViewProjection;
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
    float4 Color : COLOR0;
    float2 TexCoord : TEXCOORD0;
    int EntityID : TEXCOORD1;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR0;
    float2 TexCoord : TEXCOORD0;
    nointerpolation int EntityID : TEXCOORD1;
};

VSOutput VS_Main(VSInput input)
{
    VSOutput o;

    o.Color = input.Color;
    o.TexCoord = input.TexCoord;
    o.EntityID = u_EntityID;

    float4 worldPos = mul(float4(input.Position, 1.0f), u_Model);
    o.Position = mul(worldPos, u_ViewProjection);

    return o;
}

// ==========================
// Pixel shader resources
// ==========================

Texture2D u_FontAtlas : register(t2);
SamplerState FontSampler : register(s0);

// ==========================
// Helper functions
// ==========================

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

float screenPxRange(float2 texCoord)
{
    const float pxRange = 2.0f;

    float width, height;
    u_FontAtlas.GetDimensions(width, height);

    float2 unitRange = pxRange / float2(width, height);
    float2 screenTexSize = 1.0f / fwidth(texCoord);

    return max(0.5f * dot(unitRange, screenTexSize), 1.0f);
}

// ==========================
// Pixel shader
// ==========================

struct PSInput
{
    float4 Color : COLOR0;
    float2 TexCoord : TEXCOORD0;
    nointerpolation int EntityID : TEXCOORD1;
};

struct PSOutput
{
    float4 Color : SV_Target0;
    int EntityID : SV_Target1;
};

PSOutput PS_Main(PSInput input)
{
    PSOutput output;

    float4 texSample = u_FontAtlas.Sample(FontSampler, input.TexCoord);
    float3 msd = texSample.rgb;

    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance =
        screenPxRange(input.TexCoord) * (sd - 0.5f);

    float opacity = saturate(screenPxDistance + 0.5f);

    if (opacity == 0.0f)
        discard;

    float4 bgColor = float4(0, 0, 0, 0);
    output.Color = lerp(bgColor, input.Color, opacity);

    if (output.Color.a == 0.0f)
        discard;

    output.EntityID = input.EntityID;
    return output;
}
