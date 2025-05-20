#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
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

uniform vec3 u_ViewPos;        // Camera position in world space for lighting

void main()
{
    vec4 texColor = texture(u_Texture, v_TexCoord * u_TilingFactor);
    vec4 finalMaterialColor = u_Color * texColor;

	// Apply lighting calculations here

    color = finalMaterialColor;
}