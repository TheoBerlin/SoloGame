#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 1, binding = 2) uniform PerObject {
    mat4 WVP, World;
} g_PerObject;

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec2 in_TXCoords;

layout (location = 0) out vec3 out_Normal;
layout (location = 1) out vec3 out_WorldPos;
layout (location = 2) out vec2 out_TXCoords;

void main()
{
    vec4 inPos     = vec4(in_Position, 1.0);

    out_WorldPos    = (inPos * g_PerObject.World).xyz;
    gl_Position     = inPos * g_PerObject.WVP;
    out_Normal      = (vec4(in_Normal, 0.0) * g_PerObject.World).xyz;
    out_TXCoords    = in_TXCoords;
}
