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

#type geometry
#version 460 core

layout(triangles) in;
layout(line_strip, max_vertices = 3) out;

in vec3 v_FragPos_WorldSpace[];
in vec3 v_Normal_WorldSpace[];
in vec2 v_TexCoord[];

out vec3 g_FragPos_WorldSpace;
out vec3 g_Normal_WorldSpace;
out vec2 g_TexCoord;
noperspective out vec3 g_EdgeDistance;

uniform mat4 u_ViewProjection;

void main()
{
    vec4 p;

	p = gl_in[0].gl_Position;
    vec2 p0 = vec2(u_ViewProjection * (p / p.w));

	p = gl_in[1].gl_Position;
	vec2 p1 = vec2(u_ViewProjection * (p / p.w));

	p = gl_in[2].gl_Position;
	vec2 p2 = vec2(u_ViewProjection * (p / p.w));

	float a = length(p1 - p2);
	float b = length(p2 - p0);
	float c = length(p1 - p0);

	float alpha = acos((b * b + c * c - a * a) / (2.0 * b * c));
	float beta = acos((a * a + c * c - b * b) / (2.0 * a * c));

    float ha = abs(c * sin(beta));
	float hb = abs(c * sin(alpha));
	float hc = abs(b * sin(alpha));

    g_FragPos_WorldSpace = v_FragPos_WorldSpace[0];
    g_Normal_WorldSpace = v_Normal_WorldSpace[0];
    g_TexCoord = v_TexCoord[0];
    gl_Position = gl_in[0].gl_Position;
	g_EdgeDistance = vec3(ha, 0.0, 0.0);
    EmitVertex();

    g_FragPos_WorldSpace = v_FragPos_WorldSpace[1];
    g_Normal_WorldSpace = v_Normal_WorldSpace[1];
    g_TexCoord = v_TexCoord[1];
    gl_Position = gl_in[1].gl_Position;
    g_EdgeDistance = vec3(0.0, hb, 0.0);
    EmitVertex();

    g_FragPos_WorldSpace = v_FragPos_WorldSpace[2];
    g_Normal_WorldSpace = v_Normal_WorldSpace[2];
    g_TexCoord = v_TexCoord[2];
    gl_Position = gl_in[2].gl_Position;
    g_EdgeDistance = vec3(0.0, 0.0, hc);
    EmitVertex();

    EndPrimitive();
}

#type fragment
#version 460 core

layout(location = 0) out vec4 color;

in vec3 g_FragPos_WorldSpace;
in vec3 g_Normal_WorldSpace;
in vec2 g_TexCoord;
noperspective in vec3 g_EdgeDistance;

uniform float u_TilingFactor;
uniform sampler2D u_Texture;
uniform float u_Shininess;
uniform vec3 u_ViewPos;

uniform vec3 u_GlobalAmbientColor;
uniform float u_GlobalAmbientIntensity;

struct Material 
{
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    float shininess;
};
uniform Material u_Material;

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

    vec3 norm = normalize(g_Normal_WorldSpace);
    vec3 viewDir = normalize(u_ViewPos - g_FragPos_WorldSpace);

    // Sample material's albedo texture
    vec4 texSample = texture(u_Texture, g_TexCoord * u_TilingFactor);

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
        totalLighting += CalculatePointLight(u_PointLights[i], norm, g_FragPos_WorldSpace, viewDir, albedo);
    }

    // Calculating whether to show the wireframe
	float d = min(g_EdgeDistance.x, min(g_EdgeDistance.y, g_EdgeDistance.z));

    float mixVal = 0.0;
    float wireframeWidth = 0.5;
    if (d < wireframeWidth - 1)
    {
        mixVal = 1.0;
    }
    else if (d > wireframeWidth + 1)
    {
        mixVal = 0.0;
    }
    else
    {
		float x = d - (wireframeWidth - 1.0);
		mixVal = exp2(-2.0 * x * x);
    }

	vec4 wireframeColor = vec4(0.1, 1.0, 0.2, 1.0);

    color = vec4(totalLighting, alpha);

	color = mix(color, wireframeColor, mixVal);
}