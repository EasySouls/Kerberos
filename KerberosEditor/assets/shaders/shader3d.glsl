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

layout(location = 0) out vec3 v_FragPos_WorldSpace;
layout(location = 1) out vec3 v_Normal_WorldSpace;
layout(location = 2) out vec2 v_TexCoord;

void main()
{
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    v_FragPos_WorldSpace = worldPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(u_Model)));
    v_Normal_WorldSpace = normalize(normalMatrix * a_Normal);

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

layout(binding = 0) uniform sampler2D u_Texture;

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

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo)
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

    return (diffuse * albedo) + specular;
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

    // Directional Light
    totalLighting += CalculateDirectionalLight(u_DirectionalLight, norm, viewDir, albedo);

    // Point Lights
    for (int i = 0; i < u_NumPointLights; ++i)
    {
        totalLighting += CalculatePointLight(u_PointLights[i], norm, v_FragPos_WorldSpace, viewDir, albedo);
    }

    color = vec4(totalLighting, alpha);
    //color = vec4(albedo, 1.0);

    color2 = u_EntityID;
}