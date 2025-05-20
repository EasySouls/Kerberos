#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
//layout(location = 3) in float a_TexIndex;
//layout(location = 4) in float a_TilingFactor;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

out vec3 v_FragPos_WorldSpace;
out vec3 v_Normal_WorldSpace;
out vec2 v_TexCoord;

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
#version 460 core

layout(location = 0) out vec4 color;

in vec3 v_FragPos_WorldSpace;
in vec3 v_Normal_WorldSpace;
in vec2 v_TexCoord;

uniform vec4 u_Color;
uniform float u_TilingFactor;
uniform sampler2D u_Texture;
uniform float u_Shininess;
uniform vec3 u_ViewPos;

struct DirectionalLight
{
    bool enabled;
    vec3 direction;
    vec3 color;
    float intensity;
};
uniform DirectionalLight u_DirectionalLight;

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
uniform PointLight u_PointLights[MAX_POINT_LIGHTS];
uniform int u_NumPointLights;

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo, float specularStrength)
{
    if (!light.enabled) return vec3(0.0);

    vec3 lightDir = normalize(-light.direction);

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color * diff * light.intensity;

    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), u_Shininess);
    vec3 specular = light.color * spec * light.intensity * specularStrength;
    // Or Phong:
    // vec3 reflectDir = reflect(-lightDir, normal);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material_Shininess);
    // vec3 specular = light.color * spec * light.intensity * specularStrength;

    return (diffuse * albedo) + specular;
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float specularStrength)
{
    vec3 lightDir = normalize(light.position - fragPos);

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color * diff * light.intensity;

    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), u_Shininess);
    vec3 specular = light.color * spec * light.intensity * specularStrength;
    // Or Phong:
    // vec3 reflectDir = reflect(-lightDir, normal);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material_Shininess);
    // vec3 specular = light.color * spec * light.intensity * specularStrength;

    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    diffuse *= attenuation;
    specular *= attenuation;

    return (diffuse * albedo) + specular;
}

void main()
{
    vec3 norm = normalize(v_Normal_WorldSpace);
    vec3 viewDir = normalize(u_ViewPos - v_FragPos_WorldSpace);

    // Sample material's albedo texture
    vec4 texSample = texture(u_Texture, v_TexCoord * u_TilingFactor);
    vec3 albedo = u_Color.rgb * texSample.rgb;
    float alpha = u_Color.a * texSample.a;

    vec3 totalLighting = vec3(0.0);

    // Ambient (hardcoded for now)
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * vec3(1.0);
    totalLighting += ambient * albedo;

    // Directional Light
    float specularStrength = 0.5;
    totalLighting += CalculateDirectionalLight(u_DirectionalLight, norm, viewDir, albedo, specularStrength);

    // Point Lights
    for (int i = 0; i < u_NumPointLights; ++i)
    {
        totalLighting += CalculatePointLight(u_PointLights[i], norm, v_FragPos_WorldSpace, viewDir, albedo, specularStrength);
    }

    color = vec4(totalLighting, alpha);
}