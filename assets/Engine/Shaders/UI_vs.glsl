#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 2) uniform PerObject {
    vec2 Position, Size;
    vec4 Highlight;
    float HighlightFactor;
} g_PerObject;

layout (location = 0) in vec2 in_Position;
layout (location = 1) in vec2 in_TXCoords;

layout (location = 0) out vec2 out_TXCoords;

void main()
{
    // Resize the quad, timed by two because the size factors are in [0,1] but the vertex position
    // are expressed in [-1,1], a twice as large interval:
    // v_in.pos * size * 2
    // Translate the quad by translating whilst converting positions in [0,1] to [-1,1]:
    // + position * 2.0 - 1:
    gl_Position     = vec4(in_Position * g_PerObject.Size * 2.0 + g_PerObject.Position * 2.0 - 1.0, 0.0, 1.0);
    gl_Position.y   = -gl_Position.y;
    out_TXCoords    = in_TXCoords;
}
