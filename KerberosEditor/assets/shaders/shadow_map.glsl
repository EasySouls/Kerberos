#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

layout(std140, binding = 0) uniform Camera
{
    vec3 u_CameraPosition;
    mat4 u_ViewMatrix;
    mat4 u_ProjectionMatrix;
    mat4 u_ViewProjectionMatrix;
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
    gl_Position = u_ViewProjectionMatrix * u_Model * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

void main()
{
    // Depth is automatically written to gl_FragDepth
}