#include "RenderingHandler.hpp"

#include <Engine/ECS/Renderer.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Utils/Logger.hpp>

#include <thread>

RenderingHandler::RenderingHandler(ECSCore* pECS, Device* pDevice, Window* pWindow)
    :m_pECS(pECS),
    m_pDevice(pDevice),
    m_MeshRenderer(pECS, pDevice, pWindow),
    m_UIRenderer(pECS, pDevice, pWindow)
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
    m_pDevice->clearBackBuffer();

    updateBuffers();
    recordCommandBuffers();
    executeCommandBuffers();

    m_pDevice->presentBackBuffer();
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
