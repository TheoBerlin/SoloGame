#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 4) uniform texture2D u_UITexture;
layout(binding = 1) uniform sampler u_Sampler;

layout (binding = 2) uniform PerObject {
    vec2 Position, Size;
    vec4 Highlight;
    float HighlightFactor;
} g_PerObject;

layout (location = 0) in vec4 in_Position;
layout (location = 1) in vec2 in_TXCoords;

layout (location = 0) out vec4 out_Color;

void main()
{
    vec4 txColor    = texture(u_UITexture, in_TXCoords);
    out_Color       = min(txColor + g_PerObject.HighlightFactor * txColor * g_PerObject.Highlight, vec4(1.0));
}
