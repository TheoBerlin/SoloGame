#pragma once

// All rendering API abstractions
#include <Engine/Rendering/APIAbstractions/BlendState.hpp>
#include <Engine/Rendering/APIAbstractions/CommandPool.hpp>
#include <Engine/Rendering/APIAbstractions/DepthStencilState.hpp>
#include <Engine/Rendering/APIAbstractions/DescriptorCounts.hpp>
#include <Engine/Rendering/APIAbstractions/DescriptorPool.hpp>
#include <Engine/Rendering/APIAbstractions/DescriptorPoolHandler.hpp>
#include <Engine/Rendering/APIAbstractions/DescriptorSet.hpp>
#include <Engine/Rendering/APIAbstractions/DescriptorSetLayout.hpp>
#include <Engine/Rendering/APIAbstractions/Device.hpp>
#include <Engine/Rendering/APIAbstractions/Fence.hpp>
#include <Engine/Rendering/APIAbstractions/Framebuffer.hpp>
#include <Engine/Rendering/APIAbstractions/GeneralResources.hpp>
#include <Engine/Rendering/APIAbstractions/IBuffer.hpp>
#include <Engine/Rendering/APIAbstractions/ICommandList.hpp>
#include <Engine/Rendering/APIAbstractions/InputLayout.hpp>
#include <Engine/Rendering/APIAbstractions/IRasterizerState.hpp>
#include <Engine/Rendering/APIAbstractions/ISampler.hpp>
#include <Engine/Rendering/APIAbstractions/Pipeline.hpp>
#include <Engine/Rendering/APIAbstractions/PipelineLayout.hpp>
#include <Engine/Rendering/APIAbstractions/RenderPass.hpp>
#include <Engine/Rendering/APIAbstractions/Shader.hpp>
#include <Engine/Rendering/APIAbstractions/Texture.hpp>
#include <Engine/Rendering/APIAbstractions/Viewport.hpp>

// ECS
#include <Engine/ECS/ECSCore.hpp>

#include <Engine/Utils/Logger.hpp>

#include <string>
#include <unordered_map>
#include <unordered_set>
