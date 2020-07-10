#include "PipelineVK.hpp"

#include <Engine/Rendering/APIAbstractions/Vulkan/BlendStateVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DepthStencilStateVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/InputLayoutVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/PipelineLayoutVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/RasterizerStateVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/RenderPassVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/ShaderVK.hpp>

PipelineVK* PipelineVK::create(const PipelineInfo& pipelineInfo, DeviceVK* pDevice)
{
    std::vector<std::shared_ptr<Shader>> shaders;
    shaders.reserve(pipelineInfo.ShaderInfos.size());

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    shaderStages.reserve(shaders.size());

    VkPipelineVertexInputStateCreateInfo inputLayout = {};
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    ShaderHandler* pShaderHandler = pDevice->getShaderHandler();
    for (const ShaderInfo& shaderInfo : pipelineInfo.ShaderInfos) {
        if (shaderInfo.ShaderType == SHADER_TYPE::VERTEX_SHADER) {
            const InputLayoutInfo& inputLayoutInfo = pShaderHandler->getInputLayoutInfo(shaderInfo.ShaderName);
            if (!convertInputLayoutInfo(inputLayout, attributeDescriptions, inputLayoutInfo)) {
                return nullptr;
            }
        }

        shaders.push_back(pShaderHandler->loadShader(shaderInfo.ShaderName, shaderInfo.ShaderType));
        shaderStages.push_back(writeShaderStageInfo(shaders.back().get()));
    }

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
    inputAssemblyState.sType      = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology   = convertPrimitiveTopology(pipelineInfo.PrimitiveTopology);

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1u;
    viewportState.pViewports    = pipelineInfo.Viewports.empty() ? nullptr : (VkViewport*)pipelineInfo.Viewports.data();
    viewportState.scissorCount  = 1u;
    viewportState.pScissors     = pipelineInfo.ScissorRectangles.empty() ? nullptr : (VkRect2D*)pipelineInfo.ScissorRectangles.data();

    VkPipelineRasterizationStateCreateInfo rasterizerInfo = {};
    if (!convertRasterizerStateInfo(rasterizerInfo, pipelineInfo.RasterizerStateInfo)) {
        return nullptr;
    }

    VkPipelineMultisampleStateCreateInfo multiSampleState = {};
    multiSampleState.sType                   = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multiSampleState.rasterizationSamples    = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
    if (!convertDepthStencilStateInfo(depthStencilState, pipelineInfo.DepthStencilStateInfo)) {
        return nullptr;
    }

    VkPipelineColorBlendStateCreateInfo blendState = {};
    std::vector<VkPipelineColorBlendAttachmentState> attachmentBlendStates;
    if (!convertBlendStateInfo(blendState, attachmentBlendStates, pipelineInfo.BlendStateInfo)) {
        return nullptr;
    }

    blendState.pAttachments = attachmentBlendStates.data();

    std::vector<VkDynamicState> dynamicStates;
    dynamicStates.reserve(pipelineInfo.DynamicStates.size());
    for (PIPELINE_DYNAMIC_STATE dynamicState : pipelineInfo.DynamicStates) {
        dynamicStates.push_back(dynamicState == PIPELINE_DYNAMIC_STATE::VIEWPORT ? VK_DYNAMIC_STATE_VIEWPORT : VK_DYNAMIC_STATE_SCISSOR);
    }

    VkPipelineDynamicStateCreateInfo dynamicStatesInfo = {};
    dynamicStatesInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStatesInfo.dynamicStateCount = (uint32_t)dynamicStates.size();
    dynamicStatesInfo.pDynamicStates    = dynamicStates.data();

    VkGraphicsPipelineCreateInfo pipelineInfoVK = {};
    pipelineInfoVK.sType                = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfoVK.stageCount           = (uint32_t)pipelineInfo.ShaderInfos.size();
    pipelineInfoVK.pStages              = shaderStages.data();
    pipelineInfoVK.pVertexInputState    = &inputLayout;
    pipelineInfoVK.pInputAssemblyState  = &inputAssemblyState;
    pipelineInfoVK.pViewportState       = &viewportState;
    pipelineInfoVK.pRasterizationState  = &rasterizerInfo;
    pipelineInfoVK.pMultisampleState    = &multiSampleState;
    pipelineInfoVK.pDepthStencilState   = &depthStencilState;
    pipelineInfoVK.pColorBlendState     = &blendState;
    pipelineInfoVK.pDynamicState        = &dynamicStatesInfo;
    pipelineInfoVK.layout               = reinterpret_cast<PipelineLayoutVK*>(pipelineInfo.pLayout)->getPipelineLayout();
    pipelineInfoVK.renderPass           = reinterpret_cast<RenderPassVK*>(pipelineInfo.pRenderPass)->getRenderPass();
    pipelineInfoVK.subpass              = pipelineInfo.Subpass;

    VkPipeline pipeline = VK_NULL_HANDLE;
    if (vkCreateGraphicsPipelines(pDevice->getDevice(), VK_NULL_HANDLE, 1u, &pipelineInfoVK,  nullptr, &pipeline) != VK_SUCCESS) {
        LOG_ERROR("Failed to create graphics pipeline");
        return nullptr;
    }

    return DBG_NEW PipelineVK(pipeline, shaders, pDevice);
}

PipelineVK::PipelineVK(VkPipeline pipeline, std::vector<std::shared_ptr<Shader>>& shaders, DeviceVK* pDevice)
    :m_Pipeline(pipeline),
    m_Shaders(shaders),
    m_pDevice(pDevice)
{}

PipelineVK::~PipelineVK()
{
    vkDestroyPipeline(m_pDevice->getDevice(), m_Pipeline, nullptr);
}

VkPipelineShaderStageCreateInfo PipelineVK::writeShaderStageInfo(const Shader* pShader)
{
    const ShaderVK* pShaderVK = reinterpret_cast<const ShaderVK*>(pShader);

    VkPipelineShaderStageCreateInfo shaderInfo = {};
    shaderInfo.sType    = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderInfo.stage    = ShaderVK::convertShaderFlagBits(pShaderVK->getShaderType());
    shaderInfo.module   = pShaderVK->getShaderModule();
    shaderInfo.pName    = "main";

    return shaderInfo;
}

VkPrimitiveTopology PipelineVK::convertPrimitiveTopology(PRIMITIVE_TOPOLOGY primitiveTopology)
{
    switch (primitiveTopology) {
        case PRIMITIVE_TOPOLOGY::POINT_LIST:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PRIMITIVE_TOPOLOGY::LINE_LIST:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PRIMITIVE_TOPOLOGY::LINE_STRIP:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PRIMITIVE_TOPOLOGY::TRIANGLE_LIST:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PRIMITIVE_TOPOLOGY::TRIANGLE_STRIP:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PRIMITIVE_TOPOLOGY::LINE_LIST_WITH_ADJACENCY:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
        case PRIMITIVE_TOPOLOGY::LINE_STRIP_WITH_ADJACENCY:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
        case PRIMITIVE_TOPOLOGY::TRIANGLE_LIST_WITH_ADJACENCY:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
        case PRIMITIVE_TOPOLOGY::TRIANGLE_STRIP_WITH_ADJACENCY:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
        default:
            LOG_WARNING("Erroneous primitive topology: %d", (uint32_t)primitiveTopology);
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}
