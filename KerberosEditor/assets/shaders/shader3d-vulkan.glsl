#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
//layout(location = 3) in float a_TexIndex;
//layout(location = 4) in float a_TilingFactor;

layout (std140, set = 0, binding = 0) uniform ViewProjectionUbo {
    mat4 viewProjection;
} u_SceneData;

layout (std140, set = 1, binding = 0) uniform ModelUbo {
    mat4 model;
} u_ModelData;

layout(location = 0) out vec3 v_FragPos_WorldSpace;
layout(location = 1) out vec3 v_Normal_WorldSpace;
layout(location = 2) out vec2 v_TexCoord;

void main()
{
    vec4 worldPos = u_ModelData.model * vec4(a_Position, 1.0);
    v_FragPos_WorldSpace = worldPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(u_ModelData.model)));
    v_Normal_WorldSpace = normalize(normalMatrix * a_Normal);

    v_TexCoord = a_TexCoord;
    gl_Position = u_SceneData.viewProjection * worldPos;
}

#type fragment
#version 460 core

layout(location = 0) out vec4 color;

layout(location = 0) in vec3 v_FragPos_WorldSpace;
layout(location = 1) in vec3 v_Normal_WorldSpace;
layout(location = 2) in vec2 v_TexCoord;

struct Material 
{
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    float shininess;
};

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

#define MAX_POINT_LIGHTS 10

layout(std140, set = 0, binding = 1) uniform PerFrameData {
    vec3 viewPos;
    float dummy0;
    vec3 globalAmbientColor;
    float globalAmbientIntensity;
} u_SceneData;

layout(std140, set = 0, binding = 2) uniform DirectionalLightData {
    DirectionalLight directionalLight;
} u_DirectionalLight;

layout(std140, set = 0, binding = 3) uniform PointLightsData {
    PointLight lights[MAX_POINT_LIGHTS];
    int numPointLights;
//    float dummy3[3]; // Pad int (4 bytes) to 16 bytes (ensures the UBO ends on a 16-byte boundary or aligns next member)
} u_PointLightsData;

layout(std140, set = 1, binding = 1) uniform MaterialData {
    Material material;
    float tilingFactor;
    float dummyE[3]; // Pad float (4 bytes) to 16 bytes for overall UBO alignment
} u_MaterialData;

layout(set = 1, binding = 2) uniform sampler2D u_Texture;

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo)
{
    if (!light.enabled) return vec3(0.0);

    vec3 lightDir = normalize(-light.direction);

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color * diff * light.intensity;

    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), u_MaterialData.material.shininess);
    vec3 specular = light.color * spec * light.intensity * u_MaterialData.material.specular;

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
    float spec = pow(max(dot(normal, halfwayDir), 0.0), u_MaterialData.material.shininess);
    vec3 specular = light.color * spec * light.intensity * u_MaterialData.material.specular;

    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    diffuse *= attenuation;
    specular *= attenuation;

    return (diffuse * albedo) + specular;
}

void main()
{
    vec4 baseColor = vec4(u_MaterialData.material.diffuse, 1.0);

    vec3 norm = normalize(v_Normal_WorldSpace);
    vec3 viewDir = normalize(u_SceneData.viewPos - v_FragPos_WorldSpace);

    // Sample material's albedo texture
    vec4 texSample = texture(u_Texture, v_TexCoord * u_MaterialData.tilingFactor);

    vec3 albedo = baseColor.rgb * texSample.rgb;
    float alpha = baseColor.a * texSample.a;

    vec3 totalLighting = vec3(0);

    // Ambient
    vec3 ambient = u_SceneData.globalAmbientColor * u_SceneData.globalAmbientIntensity * u_MaterialData.material.ambient * albedo;
    totalLighting += ambient;

    // Directional Light
    totalLighting += CalculateDirectionalLight(u_DirectionalLight.directionalLight, norm, viewDir, albedo);

    // Point Lights
    for (int i = 0; i < u_PointLightsData.numPointLights; ++i)
    {
        totalLighting += CalculatePointLight(u_PointLightsData.lights[i], norm, v_FragPos_WorldSpace, viewDir, albedo);
    }

    color = vec4(totalLighting, alpha);
}