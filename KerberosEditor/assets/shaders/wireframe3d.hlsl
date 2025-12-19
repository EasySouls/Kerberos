#define MAX_POINT_LIGHTS 10

// ==========================
// Structs
// ==========================

struct Material
{
    float3 diffuse;
    float _pad0;
    float3 specular;
    float _pad1;
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
    float3 u_ViewPos;
    float _pad2;
    float4x4 u_ViewMatrix;
    float4x4 u_ProjectionMatrix;
    float4x4 u_ViewProjection;
};

cbuffer Lights : register(b1)
{
    float3 u_GlobalAmbientColor;
    float u_GlobalAmbientIntensity;

    int u_NumPointLights;
    float3 _pad3;

    DirectionalLight u_DirectionalLight;
    PointLight u_PointLights[MAX_POINT_LIGHTS];
};

cbuffer PerObjectData : register(b2)
{
    int u_EntityID;
    float3 _pad4;
    float4x4 u_Model;
    Material u_Material;
};

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

VSOutput VS_Main(VSInput input)
{
    VSOutput o;

    float4 worldPos = mul(float4(input.Position, 1.0f), u_Model);
    o.FragPos_WorldSpace = worldPos.xyz;

    float3x3 normalMatrix = transpose(Inverse3x3((float3x3) u_Model));
    o.Normal_WorldSpace = normalize(mul(input.Normal, normalMatrix));

    o.TexCoord = input.TexCoord;
    o.Position = mul(worldPos, u_ViewProjection);

    return o;
}

struct GSInput
{
    float4 Position : SV_POSITION;
    float3 FragPos_WorldSpace : TEXCOORD0;
    float3 Normal_WorldSpace : TEXCOORD1;
    float2 TexCoord : TEXCOORD2;
};

struct GSOutput
{
    float4 Position : SV_POSITION;
    float3 FragPos_WorldSpace : TEXCOORD0;
    float3 Normal_WorldSpace : TEXCOORD1;
    float2 TexCoord : TEXCOORD2;
    noperspective float3 EdgeDistance : TEXCOORD3;
};

[maxvertexcount(3)]
void GS_Main(triangle GSInput input[3], inout LineStream<GSOutput> stream)
{
    // Convert clip-space to NDC
    float2 p0 = (input[0].Position.xy / input[0].Position.w);
    float2 p1 = (input[1].Position.xy / input[1].Position.w);
    float2 p2 = (input[2].Position.xy / input[2].Position.w);

    float a = length(p1 - p2);
    float b = length(p2 - p0);
    float c = length(p1 - p0);

    float alpha = acos((b * b + c * c - a * a) / (2.0 * b * c));
    float beta = acos((a * a + c * c - b * b) / (2.0 * a * c));

    float ha = abs(c * sin(beta));
    float hb = abs(c * sin(alpha));
    float hc = abs(b * sin(alpha));

    GSOutput o;

    o = (GSOutput) 0;
    o.Position = input[0].Position;
    o.FragPos_WorldSpace = input[0].FragPos_WorldSpace;
    o.Normal_WorldSpace = input[0].Normal_WorldSpace;
    o.TexCoord = input[0].TexCoord;
    o.EdgeDistance = float3(ha, 0, 0);
    stream.Append(o);

    o.Position = input[1].Position;
    o.FragPos_WorldSpace = input[1].FragPos_WorldSpace;
    o.Normal_WorldSpace = input[1].Normal_WorldSpace;
    o.TexCoord = input[1].TexCoord;
    o.EdgeDistance = float3(0, hb, 0);
    stream.Append(o);

    o.Position = input[2].Position;
    o.FragPos_WorldSpace = input[2].FragPos_WorldSpace;
    o.Normal_WorldSpace = input[2].Normal_WorldSpace;
    o.TexCoord = input[2].TexCoord;
    o.EdgeDistance = float3(0, 0, hc);
    stream.Append(o);

    stream.RestartStrip();
}

Texture2D u_Texture : register(t0);
SamplerState LinearSampler : register(s0);

struct PSInput
{
    float3 FragPos_WorldSpace : TEXCOORD0;
    float3 Normal_WorldSpace : TEXCOORD1;
    float2 TexCoord : TEXCOORD2;
    noperspective float3 EdgeDistance : TEXCOORD3;
};

struct PSOutput
{
    float4 Color : SV_Target0;
    int EntityID : SV_Target1;
};

float3 CalculateDirectionalLight(
    DirectionalLight light,
    float3 normal,
    float3 viewDir,
    float3 albedo)
{
    if (light.enabled == 0)
        return 0.0f;

    float3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0f);
    float3 diffuse = light.color * diff * light.intensity;

    float3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0f), u_Material.shininess);
    float3 specular = light.color * spec * light.intensity * u_Material.specular;

    return diffuse * albedo + specular;
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

    float dist = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.lin * dist + light.quadratic * dist * dist);

    return (diffuse * albedo + specular) * attenuation;
}

PSOutput PS_Main(PSInput input)
{
    PSOutput o;

    float3 norm = normalize(input.Normal_WorldSpace);
    float3 viewDir = normalize(u_ViewPos - input.FragPos_WorldSpace);

    float4 texSample = u_Texture.Sample(LinearSampler, input.TexCoord);
    float3 albedo = u_Material.diffuse * texSample.rgb;
    float alpha = texSample.a;

    float3 lighting = 0.0f;

    lighting += u_GlobalAmbientColor *
                u_GlobalAmbientIntensity *
                u_Material.ambient *
                albedo;

    lighting += CalculateDirectionalLight(
        u_DirectionalLight, norm, viewDir, albedo);

    for (int i = 0; i < u_NumPointLights; ++i)
    {
        lighting += CalculatePointLight(
            u_PointLights[i], norm,
            input.FragPos_WorldSpace, viewDir, albedo);
    }

    // Wireframe logic
    float d = min(input.EdgeDistance.x,
              min(input.EdgeDistance.y, input.EdgeDistance.z));

    float wireframeWidth = 0.5f;
    float mixVal;

    if (d < wireframeWidth - 1)
        mixVal = 1.0f;
    else if (d > wireframeWidth + 1)
        mixVal = 0.0f;
    else
    {
        float x = d - (wireframeWidth - 1.0f);
        mixVal = exp2(-2.0f * x * x);
    }

    float4 wireColor = float4(0.1f, 1.0f, 0.2f, 1.0f);
    float4 baseColor = float4(lighting, alpha);

    o.Color = lerp(baseColor, wireColor, mixVal);
    o.EntityID = u_EntityID;

    return o;
}
