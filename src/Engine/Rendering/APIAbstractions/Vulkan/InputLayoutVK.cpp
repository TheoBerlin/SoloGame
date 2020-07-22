#include "InputLayoutVK.hpp"

#include <Engine/Rendering/APIAbstractions/GeneralResources.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/GeneralResourcesVK.hpp>

bool convertInputLayoutInfo(InputLayoutInfoVK& inputLayoutInfoVK, const InputLayoutInfo& inputLayoutInfo)
{
    uint32_t vertexOffset   = 0u;
    uint32_t location       = 0u;

    inputLayoutInfoVK.AttributeDescriptions.reserve(inputLayoutInfo.VertexInputAttributes.size());
    for (const InputVertexAttribute& vertexAttribute : inputLayoutInfo.VertexInputAttributes) {
        VkVertexInputAttributeDescription attributeDesc = {};
        attributeDesc.location  = location++;
        attributeDesc.binding   = 0u;
        attributeDesc.format    = convertFormatToVK(vertexAttribute.Format);
        attributeDesc.offset    = vertexOffset;
        inputLayoutInfoVK.AttributeDescriptions.push_back(attributeDesc);

        vertexOffset += (uint32_t)getFormatSize(vertexAttribute.Format);
    }

    inputLayoutInfoVK.InputBindingDescription = {};
    inputLayoutInfoVK.InputBindingDescription.binding     = inputLayoutInfo.Binding;
    inputLayoutInfoVK.InputBindingDescription.stride      = vertexOffset;
    inputLayoutInfoVK.InputBindingDescription.inputRate   = inputLayoutInfo.InputRate == VERTEX_INPUT_RATE::PER_VERTEX ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;

    inputLayoutInfoVK.VertexInputState = {};
    inputLayoutInfoVK.VertexInputState.sType                             = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    inputLayoutInfoVK.VertexInputState.vertexBindingDescriptionCount     = 1u;
    inputLayoutInfoVK.VertexInputState.pVertexBindingDescriptions        = &inputLayoutInfoVK.InputBindingDescription;
    inputLayoutInfoVK.VertexInputState.vertexAttributeDescriptionCount   = (uint32_t)inputLayoutInfoVK.AttributeDescriptions.size();
    inputLayoutInfoVK.VertexInputState.pVertexAttributeDescriptions      = inputLayoutInfoVK.AttributeDescriptions.data();

    return true;
}
