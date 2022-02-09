#include "EngineCore.hpp"

#include <vendor/json/json.hpp>

#include <fstream>

EngineCore* EngineCore::s_pInstance = nullptr;

EngineCore::EngineCore()
    :   m_pPhysicsCore(nullptr)
    ,   m_pAssetLoaders(nullptr)
    ,   m_pUICore(nullptr)
    ,   m_pRenderingCore(nullptr)
    ,   m_pAudioCore(nullptr)
{}

EngineCore::~EngineCore()
{
    delete m_pPhysicsCore;
    delete m_pAssetLoaders;
    delete m_pUICore;
    delete m_pAudioCore;
    delete m_pRenderingCore;
}

bool EngineCore::Init()
{
    EngineConfig engineCFG;
    if (!LoadEngineConfig(engineCFG)) {
        return false;
    }

    m_pRenderingCore = DBG_NEW RenderingCore();
    if (!m_pRenderingCore->Init(engineCFG)) {
        return false;
    }

    m_pPhysicsCore  = DBG_NEW PhysicsCore();
    m_pAssetLoaders = DBG_NEW AssetLoadersCore(m_pRenderingCore);

    m_pUICore = DBG_NEW UICore(m_pRenderingCore);
    if (!m_pUICore->Init()) {
        return false;
    }

    m_pAudioCore = DBG_NEW AudioCore();
    return m_pAudioCore->Init();
}

bool EngineCore::LoadEngineConfig(EngineConfig& engineConfig) const
{
    // Default config
    engineConfig.RenderingAPI       = RENDERING_API::VULKAN;
    engineConfig.PresentationMode   = PRESENTATION_MODE::MAILBOX;

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
        [](unsigned char c){ return (char)std::tolower(c); });

    const std::unordered_set<std::string> dx11Strings = {
        "dx11", "directx11", "directx 11"
    };

    engineConfig.RenderingAPI = dx11Strings.contains(APIStr) ? RENDERING_API::DIRECTX11 : RENDERING_API::VULKAN;

    std::string presentationModeStr = configJSON["PresentationMode"].get<std::string>();
    std::transform(presentationModeStr.begin(), presentationModeStr.end(), presentationModeStr.begin(),
        [](unsigned char c){ return (char)std::tolower(c); });

    if (presentationModeStr == "immediate") {
        engineConfig.PresentationMode = PRESENTATION_MODE::IMMEDIATE;
    }

    return true;
}
