#include "InputLayoutVK.hpp"

#include <Engine/Rendering/APIAbstractions/GeneralResources.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/GeneralResourcesVK.hpp>

bool convertInputLayoutInfo(VkPipelineVertexInputStateCreateInfo& inputLayoutInfoVK, const InputLayoutInfo& inputLayoutInfo)
{
    uint32_t vertexOffset   = 0u;
    uint32_t location       = 0u;

    std::vector<VkVertexInputAttributeDescription> attributeDescs;
    attributeDescs.reserve(inputLayoutInfo.VertexInputAttributes.size());
    for (const InputVertexAttribute& vertexAttribute : inputLayoutInfo.VertexInputAttributes) {
        vertexOffset += (uint32_t)getFormatSize(vertexAttribute.Format);

        VkVertexInputAttributeDescription attributeDesc = {};
        attributeDesc.location  = location++;
        attributeDesc.binding   = 0u;
        attributeDesc.format    = convertFormatToVK(vertexAttribute.Format);
        attributeDesc.offset    = vertexOffset;
        attributeDescs.push_back(attributeDesc);
    }

    VkVertexInputBindingDescription bindingDesc = {};
    bindingDesc.binding     = inputLayoutInfo.Binding;
    bindingDesc.stride      = vertexOffset;
    bindingDesc.inputRate   = inputLayoutInfo.InputRate == VERTEX_INPUT_RATE::PER_VERTEX ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;

    inputLayoutInfoVK = {};
    inputLayoutInfoVK.sType                             = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    inputLayoutInfoVK.vertexBindingDescriptionCount     = 1u;
    inputLayoutInfoVK.pVertexBindingDescriptions        = &bindingDesc;
    inputLayoutInfoVK.vertexAttributeDescriptionCount   = (uint32_t)attributeDescs.size();
    inputLayoutInfoVK.pVertexAttributeDescriptions      = attributeDescs.data();

    return true;
}
