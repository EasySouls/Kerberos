#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;

layout(location = 0) out vec3 v_TexCoords;

layout(std140, binding = 0) uniform CameraData
{
    vec3 u_ViewPos;
    mat4 u_View;
    mat4 u_Projection;
    mat4 u_ViewProjection;
};

void main()
{
    v_TexCoords = a_Position;

    vec4 pos = u_Projection * u_View * vec4(a_Position, 1.0);
    gl_Position = pos.xyww;
}

#type fragment
#version 460 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out int EntityIDColor;

layout(location = 0) in vec3 v_TexCoords;

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

layout(binding = 0) uniform samplerCube u_Skybox;

void main()
{
    FragColor = texture(u_Skybox, v_TexCoords);

    // We could just output a constant -1 instead of taking in a uniform then passing
    // it to the fragment shader, but it might come in handy later
    EntityIDColor = u_EntityID;
}