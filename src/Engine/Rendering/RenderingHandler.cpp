#include "RenderingHandler.hpp"

#include <Engine/ECS/Renderer.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Utils/Logger.hpp>

#include <thread>

RenderingHandler::RenderingHandler(ECSCore* pECS, Device* pDevice)
    :m_pECS(pECS),
    m_pDevice(pDevice),
    m_MeshRenderer(pECS, pDevice),
    m_UIRenderer(pECS, pDevice)
{}

RenderingHandler::~RenderingHandler()
{}

bool RenderingHandler::init()
{
    m_Renderers = {
        &m_MeshRenderer,
        &m_UIRenderer
    };

    return true;
}

void RenderingHandler::render()
{
    Swapchain* pSwapchain = m_pDevice->getSwapchain();
    uint32_t& frameIndex = m_pDevice->getFrameIndex();
    pSwapchain->acquireNextBackbuffer(frameIndex, SYNC_OPTION::FENCE);

    IFence* pBackbufferReadyFence = pSwapchain->getFence();
    m_pDevice->waitForFences(&pBackbufferReadyFence, 1u, false, UINT64_MAX);

    updateBuffers();
    recordCommandBuffers();
    executeCommandBuffers();

    pSwapchain->present(nullptr, 0u);
}

void RenderingHandler::updateBuffers()
{
    std::vector<std::thread> threads;
    threads.reserve(m_Renderers.size());

    for (Renderer* pRenderer : m_Renderers) {
        threads.push_back(std::thread(&Renderer::updateBuffers, pRenderer));
    }

    for (std::thread& thread : threads) {
        thread.join();
    }
}

void RenderingHandler::recordCommandBuffers()
{
    std::vector<std::thread> threads;
    threads.reserve(m_Renderers.size());

    for (Renderer* pRenderer : m_Renderers) {
        threads.push_back(std::thread(&Renderer::recordCommands, pRenderer));
    }

    for (std::thread& thread : threads) {
        thread.join();
    }
}

void RenderingHandler::executeCommandBuffers()
{
    m_MeshRenderer.executeCommands();
    m_UIRenderer.executeCommands();
}
