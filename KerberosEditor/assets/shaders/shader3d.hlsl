// ==========================
// Common definitions
// ==========================

#define MAX_POINT_LIGHTS 10

struct Material
{
    float3 diffuse;
    float3 specular;
    float3 ambient;
    float shininess;
};

struct DirectionalLight
{
    int enabled;
    float3 direction;
    float3 color;
    float intensity;
};

struct PointLight
{
    float3 position;
    float3 color;
    float intensity;

    float constant;
    float lin;
    float quadratic;
};

// ==========================
// Constant buffers
// ==========================

cbuffer Camera : register(b0)
{
    float3 u_CameraPosition; // VS
    float _pad0;
    float4x4 u_ViewMatrix;
    float4x4 u_ProjectionMatrix;
    float4x4 u_ViewProjection;
};

cbuffer Lights : register(b1)
{
    float3 u_GlobalAmbientColor;
    float u_GlobalAmbientIntensity;

    int u_NumPointLights;
    float3 _pad1;

    DirectionalLight u_DirectionalLight;
    PointLight u_PointLights[MAX_POINT_LIGHTS];
};

cbuffer PerObjectData : register(b2)
{
    int u_EntityID;
    float3 _pad2;
    float4x4 u_Model;
    Material u_Material;
};

cbuffer ShadowData : register(b3)
{
    float4x4 u_LightSpaceMatrix;
    int u_EnableShadows;
    float u_ShadowBias;
    float2 _pad3;
};

float3x3 Inverse3x3(float3x3 m)
{
    float3 a = m[0];
    float3 b = m[1];
    float3 c = m[2];

    float3 r0 = cross(b, c);
    float3 r1 = cross(c, a);
    float3 r2 = cross(a, b);

    float invDet = 1.0 / dot(r2, c);

    return float3x3(
        r0 * invDet,
        r1 * invDet,
        r2 * invDet
    );
}

// ==========================
// Vertex shader
// ==========================

struct VSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float3 FragPos_WorldSpace : TEXCOORD0;
    float3 Normal_WorldSpace : TEXCOORD1;
    float2 TexCoord : TEXCOORD2;
    float4 FragPos_LightSpace : TEXCOORD3;
};

VSOutput VS_Main(VSInput input)
{
    VSOutput o;

    float4 worldPos = mul(float4(input.Position, 1.0f), u_Model);
    o.FragPos_WorldSpace = worldPos.xyz;

    float3x3 normalMatrix = transpose(Inverse3x3((float3x3) u_Model));
    o.Normal_WorldSpace = normalize(mul(input.Normal, normalMatrix));

    o.FragPos_LightSpace = mul(float4(o.FragPos_WorldSpace, 1.0f), u_LightSpaceMatrix);
    o.TexCoord = input.TexCoord;

    o.Position = mul(worldPos, u_ViewProjection);
    return o;
}

// ==========================
// Pixel shader resources
// ==========================

Texture2D u_Texture : register(t0);
Texture2D u_ShadowMap : register(t1);
SamplerState LinearSampler : register(s0);

// ==========================
// Helper functions
// ==========================

float ShadowCalculation(float4 fragPosLightSpace, float3 normal, float3 lightDir)
{
    float3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5f + 0.5f;

    if (projCoords.z > 1.0f)
        return 0.0f;

    float currentDepth = projCoords.z;
    float bias = u_ShadowBias;

    float shadow = 0.0f;
    float2 texelSize;
    u_ShadowMap.GetDimensions(texelSize.x, texelSize.y);
    texelSize = 1.0f / texelSize;

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth =
                u_ShadowMap.Sample(LinearSampler, projCoords.xy + float2(x, y) * texelSize).r;

            shadow += (currentDepth - bias > pcfDepth) ? 1.0f : 0.0f;
        }
    }

    return shadow / 9.0f;
}

float3 CalculateDirectionalLight(
    DirectionalLight light,
    float3 normal,
    float3 viewDir,
    float3 albedo,
    float shadow)
{
    if (light.enabled == 0)
        return 0.0f;

    float3 lightDir = normalize(-light.direction);

    float diff = max(dot(normal, lightDir), 0.0f);
    float3 diffuse = light.color * diff * light.intensity;

    float3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0f), u_Material.shininess);
    float3 specular = light.color * spec * light.intensity * u_Material.specular;

    return ((diffuse * albedo) + specular) * (1.0f - shadow);
}

float3 CalculatePointLight(
    PointLight light,
    float3 normal,
    float3 fragPos,
    float3 viewDir,
    float3 albedo)
{
    float3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0f);
    float3 diffuse = light.color * diff * light.intensity;

    float3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0f), u_Material.shininess);
    float3 specular = light.color * spec * light.intensity * u_Material.specular;

    float distance = length(light.position - fragPos);
    float attenuation =
        1.0f / (light.constant + light.lin * distance + light.quadratic * distance * distance);

    return (diffuse * albedo + specular) * attenuation;
}

// ==========================
// Pixel shader
// ==========================

struct PSInput
{
    float3 FragPos_WorldSpace : TEXCOORD0;
    float3 Normal_WorldSpace : TEXCOORD1;
    float2 TexCoord : TEXCOORD2;
    float4 FragPos_LightSpace : TEXCOORD3;
};

struct PSOutput
{
    float4 Color : SV_Target0;
    int Color2 : SV_Target1;
};

PSOutput PS_Main(PSInput input)
{
    PSOutput o;

    float3 norm = normalize(input.Normal_WorldSpace);
    float3 viewDir = normalize(u_CameraPosition - input.FragPos_WorldSpace);

    float4 texSample = u_Texture.Sample(LinearSampler, input.TexCoord);

    float3 albedo = u_Material.diffuse * texSample.rgb;
    float alpha = texSample.a;

    float3 totalLighting = 0.0f;

    // Ambient
    totalLighting +=
        u_GlobalAmbientColor *
        u_GlobalAmbientIntensity *
        u_Material.ambient *
        albedo;

    // Shadows
    float shadow = 0.0f;
    if (u_EnableShadows == 1 && u_DirectionalLight.enabled == 1)
    {
        shadow = ShadowCalculation(input.FragPos_LightSpace, norm,
                                   normalize(u_DirectionalLight.direction));
    }

    totalLighting +=
        CalculateDirectionalLight(u_DirectionalLight, norm, viewDir, albedo, shadow);

    for (int i = 0; i < u_NumPointLights; ++i)
    {
        totalLighting +=
            CalculatePointLight(u_PointLights[i], norm,
                                input.FragPos_WorldSpace, viewDir, albedo);
    }

    o.Color = float4(totalLighting, alpha);
    o.Color2 = u_EntityID;
    return o;
}
