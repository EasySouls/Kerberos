#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

layout(binding = 0) uniform CameraBuffer
{
	mat4 u_ViewProjection;
	mat4 u_Transform;
};

layout(location = 0) out vec3 v_Position;

void main()
{
	v_Position = a_Position;
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) in vec3 v_Position;

layout(binding = 0) uniform ColorBuffer
{
	vec4 u_Color;
};

layout(location = 0) out vec4 color;

void main()
{
	color = u_Color;
}