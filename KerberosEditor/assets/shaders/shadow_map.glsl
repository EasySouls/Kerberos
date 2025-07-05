#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

layout(std140, binding = 3) uniform ShadowData
{
    mat4 u_LightSpaceMatrix;
    int u_EnableShadows;
    float u_ShadowBias;
};

struct Material
{
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    float shininess;
};

layout(std140, binding = 2) uniform PerObjectData
{
    int u_EntityID;
    mat4 u_Model;
    Material u_Material;
};

void main()
{
    gl_Position = u_LightSpaceMatrix * u_Model * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

void main()
{
    // Depth is automatically written to gl_FragDepth by the way
	gl_FragDepth = gl_FragCoord.z;
}