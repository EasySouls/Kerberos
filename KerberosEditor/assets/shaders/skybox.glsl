#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;

out vec3 TexCoords;

uniform mat4 u_Projection;
uniform mat4 u_View;

void main()
{
    TexCoords = a_Position;

    vec4 pos = u_Projection * u_View * vec4(a_Position, 1.0);
    gl_Position = pos.xyww;
}

#type fragment
#version 460 core

out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube u_Skybox;

void main()
{
    FragColor = texture(u_Skybox, TexCoords);
}