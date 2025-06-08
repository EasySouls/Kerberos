#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;

layout(location = 0) out vec3 v_TexCoords;
layout(location = 1) out flat int v_EntityID;

layout(binding = 0) uniform CameraData
{
    vec3 u_ViewPos;
    mat4 u_View;
    mat4 u_Projection;
    mat4 u_ViewProjection;
};

layout(push_constant) uniform EntityData
{
    int u_EntityID;
};

void main()
{
    v_TexCoords = a_Position;
    v_EntityID = u_EntityID;

    vec4 pos = u_Projection * u_View * vec4(a_Position, 1.0);
    gl_Position = pos.xyww;
}

#type fragment
#version 460 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out int EntityIDColor;

layout(location = 0) in vec3 v_TexCoords;
layout(location = 1) in flat int v_EntityID;

layout(binding = 0) uniform samplerCube u_Skybox;

void main()
{
    FragColor = texture(u_Skybox, v_TexCoords);

    // We could just output a constant -1 instead of taking in a uniform then passing
    // it to the fragment shader, but it might come in handy later
    EntityIDColor = v_EntityID;
}