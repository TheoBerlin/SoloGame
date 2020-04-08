#include "RenderingHandler.hpp"

#include <Engine/ECS/Renderer.hpp>
#include <Engine/Rendering/Display.hpp>
#include <Engine/Utils/Logger.hpp>

#include <thread>

RenderingHandler::RenderingHandler(ECSCore* pECS, Display* pDisplay)
    :m_pECS(pECS),
    m_pDisplay(pDisplay),
    m_MeshRenderer(pECS, pDisplay),
    m_UIRenderer(pECS, pDisplay)
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
    m_pDisplay->clearBackBuffer();

    recordCommandBuffers();
    executeCommandBuffers();

    m_pDisplay->presentBackBuffer();
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
