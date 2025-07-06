#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
//layout(location = 3) in float a_TexIndex;
//layout(location = 4) in float a_TilingFactor;

layout(std140, binding = 0) uniform Camera
{
	vec3 u_CameraPosition;
    mat4 u_ViewMatrix;
	mat4 u_ProjectionMatrix;
    mat4 u_ViewProjection;
};

struct Material
{
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    float shininess;
};

//layout(push_constant) uniform PerObjectData {
//    mat4 u_Model;
//    Material u_Material;
//    int u_EntityID;
//};
layout(std140, binding = 2) uniform PerObjectData
{
    int u_EntityID;
    mat4 u_Model;
    Material u_Material;
};

layout(std140, binding = 3) uniform ShadowData
{
    mat4 u_LightSpaceMatrix;
    int u_EnableShadows;
    float u_ShadowBias;
};

layout(location = 0) out vec3 v_FragPos_WorldSpace;
layout(location = 1) out vec3 v_Normal_WorldSpace;
layout(location = 2) out vec2 v_TexCoord;
layout(location = 3) out vec4 v_FragPos_LightSpace;

void main()
{
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    v_FragPos_WorldSpace = worldPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(u_Model)));
    v_Normal_WorldSpace = normalize(normalMatrix * a_Normal);

	v_FragPos_LightSpace = u_LightSpaceMatrix * vec4(v_FragPos_WorldSpace, 1.0);

    v_TexCoord = a_TexCoord;
    gl_Position = u_ViewProjection * worldPos;
}

#type fragment
#version 450 core

layout(std140, binding = 0) uniform Camera
{
    vec3 u_ViewPos;
    mat4 u_ViewMatrix;
    mat4 u_ProjectionMatrix;
    mat4 u_ViewProjection;
};

layout(location = 0) out vec4 color;
layout(location = 1) out int color2;

layout(location = 0) in vec3 v_FragPos_WorldSpace;
layout(location = 1) in vec3 v_Normal_WorldSpace;
layout(location = 2) in vec2 v_TexCoord;
layout(location = 3) in vec4 v_FragPos_LightSpace;

layout(binding = 0) uniform sampler2D u_Texture;
layout(binding = 1) uniform sampler2D u_ShadowMap;

#define MAX_POINT_LIGHTS 10

struct DirectionalLight
{
    bool enabled;
    vec3 direction;
    vec3 color;
    float intensity;
};

struct PointLight
{
    vec3 position;
    vec3 color;
    float intensity;

    float constant;
    float linear;
    float quadratic;
};

layout(std140, binding = 1) uniform Lights
{
    vec3 u_GlobalAmbientColor;
    float u_GlobalAmbientIntensity;

    int u_NumPointLights;
    DirectionalLight u_DirectionalLight;
    PointLight u_PointLights[MAX_POINT_LIGHTS];
};

struct Material
{
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    float shininess;
};

//layout(push_constant) uniform PerObjectData
//{
//    mat4 u_Model;
//    Material u_Material;
//    int u_EntityID;
//};
layout(std140, binding = 2) uniform PerObjectData
{
    int u_EntityID;
    mat4 u_Model;
    Material u_Material;
};

layout(std140, binding = 3) uniform ShadowData
{
    mat4 u_LightSpaceMatrix;
    int u_EnableShadows;
    float u_ShadowBias;
};

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // Perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
        return 0.0;

    float closestDepth = texture(u_ShadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    // Bias to prevent shadow acne
    //float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	float bias = 0.005;

    // PCF (Percentage Closer Filtering)
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(u_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo, float shadow)
{
    if (!light.enabled) return vec3(0.0);

    vec3 lightDir = normalize(-light.direction);

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color * diff * light.intensity;

    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), u_Material.shininess);
    vec3 specular = light.color * spec * light.intensity * u_Material.specular;

    return ((diffuse * albedo) + specular) * (1.0 - shadow);
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo)
{
    vec3 lightDir = normalize(light.position - fragPos);

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color * diff * light.intensity;

    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), u_Material.shininess);
    vec3 specular = light.color * spec * light.intensity * u_Material.specular;

    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    diffuse *= attenuation;
    specular *= attenuation;

    return (diffuse * albedo) + specular;
}

void main()
{
    vec4 baseColor = vec4(u_Material.diffuse, 1.0);

    vec3 norm = normalize(v_Normal_WorldSpace);
    vec3 viewDir = normalize(u_ViewPos - v_FragPos_WorldSpace);

    // Sample material's albedo texture
    //vec4 texSample = texture(u_Texture, v_TexCoord * u_TilingFactor);
    vec4 texSample = texture(u_Texture, v_TexCoord * 1.0);

    vec3 albedo = baseColor.rgb * texSample.rgb;
    float alpha = baseColor.a * texSample.a;

    vec3 totalLighting = vec3(0);

    // Ambient
    vec3 ambient = u_GlobalAmbientColor * u_GlobalAmbientIntensity * u_Material.ambient * albedo;
    totalLighting += ambient;

	// Shadow calculation
    float shadow = 0.0;
    if (u_EnableShadows == 1 && u_DirectionalLight.enabled)
    {
        vec3 lightDir = normalize(u_DirectionalLight.direction);
        shadow = ShadowCalculation(v_FragPos_LightSpace, norm, lightDir);
	}

    // Directional Light
    totalLighting += CalculateDirectionalLight(u_DirectionalLight, norm, viewDir, albedo, shadow);

    // Point Lights
    for (int i = 0; i < u_NumPointLights; ++i)
    {
        totalLighting += CalculatePointLight(u_PointLights[i], norm, v_FragPos_WorldSpace, viewDir, albedo);
    }

    color = vec4(totalLighting, alpha);
    //color = vec4(shadow, 0.0, 0.0, 1.0);

    color2 = u_EntityID;
}