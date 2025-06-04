#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec4 a_Color;

layout(location = 0) out vec4 v_FragColor;

void main()
{
	v_FragColor = a_Color;
    gl_Position = vec4(a_Position, 1.0f);
}

#type fragment
#version 460 core

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec4 v_FragColor;

void main()
{
    outColor = v_FragColor;
}