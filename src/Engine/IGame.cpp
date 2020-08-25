#include "IGame.hpp"

#include <Engine/Utils/Debug.hpp>

#include <vendor/json/json.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>

IGame::IGame()
    :m_Window(720u, 16.0f / 9.0f, true),
    m_pDevice(nullptr),
    m_StateManager(&m_ECS),
    m_pPhysicsCore(nullptr),
    m_pAssetLoaders(nullptr),
    m_pUICore(nullptr),
    m_pRenderingCore(nullptr),
    m_pAudioCore(nullptr),
    m_pRenderingHandler(nullptr)
{}

IGame::~IGame()
{
    if (m_pDevice) {
        m_pDevice->waitIdle();
    }

    delete m_pPhysicsCore;
    delete m_pAssetLoaders;
    delete m_pUICore;
    delete m_pRenderingCore;
    delete m_pAudioCore;

    delete m_pRenderingHandler;
    delete m_pDevice;
}

bool IGame::init()
{
    EngineConfig engineCFG = {};
    if (!loadEngineConfig(engineCFG)) {
        return false;
    }
    LOG_INFOF("Launching with %s", engineCFG.RenderingAPI == RENDERING_API::VULKAN ? "Vulkan" : "DirectX 11");

    if (!m_Window.init()) {
        return false;
    }

    SwapchainInfo swapChainInfo = {};
    swapChainInfo.FrameRateLimit    = 60u;
    swapChainInfo.Multisamples      = 1u;
    swapChainInfo.Windowed          = true;

    DescriptorCounts descriptorPoolSize;
    descriptorPoolSize.setAll(100u);

    m_pDevice = Device::create(engineCFG.RenderingAPI, swapChainInfo, &m_Window);
    if (!m_pDevice || !m_pDevice->init(descriptorPoolSize)) {
        return false;
    }

    m_pPhysicsCore      = DBG_NEW PhysicsCore(&m_ECS);
    m_pAssetLoaders     = DBG_NEW AssetLoadersCore(&m_ECS, m_pDevice);
    m_pUICore           = DBG_NEW UICore(&m_ECS, m_pDevice, &m_Window);
    m_pRenderingCore    = DBG_NEW RenderingCore(&m_ECS, m_pDevice, &m_Window);
    m_pAudioCore        = DBG_NEW AudioCore(&m_ECS);

    m_pRenderingHandler = DBG_NEW RenderingHandler(&m_ECS, m_pDevice);
    if (!m_pRenderingHandler->init()) {
        return false;
    }

    m_ECS.performRegistrations();
    m_Window.show();

    return true;
}

void IGame::run()
{
    auto timer = std::chrono::high_resolution_clock::now();
    auto timeNow = timer;
    std::chrono::duration<float> dtChrono;

    MSG msg = {0};
    while(!m_Window.shouldClose()) {
        m_Window.pollEvents();

        timeNow = std::chrono::high_resolution_clock::now();
        dtChrono = timeNow - timer;
        float dt = dtChrono.count();

        timer = timeNow;
        m_RuntimeStats.setFrameTime(dt);

        m_Window.getInputHandler()->update();

        // Update logic
        m_ECS.update(dt);
        m_StateManager.update(dt);

        m_pRenderingHandler->render();
    }
}

bool IGame::loadEngineConfig(EngineConfig& engineConfig) const
{
    // Default config
    engineConfig.RenderingAPI = RENDERING_API::VULKAN;

    using json = nlohmann::json;

    const char* pInFile = "engine_config.json";
    std::ifstream cfgFile(pInFile);
    if (!cfgFile.is_open()) {
        LOG_ERRORF("Failed to open engine configuration file: %s", pInFile);
        return false;
    }

    json configJSON;
    cfgFile >> configJSON;
    cfgFile.close();

    std::string APIStr = configJSON["API"].get<std::string>();
    std::transform(APIStr.begin(), APIStr.end(), APIStr.begin(),
        [](unsigned char c){ return std::tolower(c); });

    const std::unordered_set<std::string> dx11Strings = {
        "dx11", "directx11", "directx 11"
    };

    engineConfig.RenderingAPI = dx11Strings.contains(APIStr) ? RENDERING_API::DIRECTX11 : RENDERING_API::VULKAN;
    return true;
}
