#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 4) uniform sampler2D u_UITexture;

layout (binding = 2) uniform PerObject {
    vec2 Position, Size;
    vec4 Highlight;
    float HighlightFactor;
} g_PerObject;

layout (location = 0) in vec2 in_TXCoords;

layout (location = 0) out vec4 out_Color;

void main()
{
    vec4 txColor    = texture(u_UITexture, in_TXCoords);
    out_Color       = clamp(txColor + g_PerObject.HighlightFactor * txColor * g_PerObject.Highlight, 0.0, 1.0);
}
