#include "MainMenuState.hpp"

#include <Engine/Audio/SoundPlayer.hpp>
#include <Engine/ECS/ECSCore.hpp>
#include <Engine/GameState/StateManager.hpp>
#include <Engine/InputHandler.hpp>
#include <Engine/Rendering/AssetLoaders/TextureCache.hpp>
#include <Engine/Rendering/Text/TextRenderer.hpp>
#include <Engine/UI/Panel.hpp>
#include <Engine/Utils/Debug.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/Logger.hpp>
#include <Game/States/GameSession.hpp>

#include <array>

MainMenuState::MainMenuState(StateManager* pStateManager)
    :State(pStateManager)
{}

void MainMenuState::Init()
{
    EngineCore::GetInstance()->GetRenderingCore()->GetWindow()->GetInputHandler()->showCursor();

    CreatePlayButton();

    LOG_INFO("Entered main menu, press E to start a game session");
}

void MainMenuState::Resume()
{
    EngineCore::GetInstance()->GetRenderingCore()->GetWindow()->GetInputHandler()->showCursor();
}

void MainMenuState::Pause()
{}

void MainMenuState::Update(float dt)
{
    UNREFERENCED_VARIABLE(dt);

    if (EngineCore::GetInstance()->GetRenderingCore()->GetWindow()->GetInputHandler()->KeyState(GLFW_KEY_E)) {
        createGameSession();
    }
}

void MainMenuState::CreatePlayButton()
{
    constexpr const DirectX::XMFLOAT2 buttonPos         = { 0.4f, 0.45f };
    constexpr const DirectX::XMFLOAT2 buttonSize        = { 0.2f, 0.1f };
    constexpr const DirectX::XMFLOAT4 buttonHighlight   = { 0.0f, 0.0f, 0.0f, 0.0f };
    constexpr const float buttonHighlightFactor         = 1.0f;

    ECSCore* pECS = ECSCore::GetInstance();
    EngineCore* pEngineCore = EngineCore::GetInstance();
    UICore* pUICore = pEngineCore->GetUICore();

    UIHandler* pUIHandler = pUICore->GetPanelHandler();

    m_PlayButtonEntity = pECS->CreateEntity();
    pECS->AddComponent(m_PlayButtonEntity, pUIHandler->CreatePanel(buttonPos, buttonSize, buttonHighlight, buttonHighlightFactor));

    // Attach background and text textures to the panel
    TextureCache* pTextureCache = pEngineCore->GetAssetLoadersCore()->GetTextureCache();
    TextRenderer* pTextRenderer = pUICore->GetTextRenderer();

    const std::array<std::shared_ptr<Texture>, 2> panelTextures = {
        pTextureCache->LoadTexture("./assets/Models/Cube.png"),
        pTextRenderer->renderText("Play", "assets/Fonts/arial/arial.ttf", 50)
    };

    TextureAttachmentInfo txAttachmentInfos[2];
    txAttachmentInfos[0].sizeSetting            = TX_SIZE_STRETCH;
    txAttachmentInfos[1].horizontalAlignment    = TX_HORIZONTAL_ALIGNMENT_CENTER;
    txAttachmentInfos[1].verticalAlignment      = TX_VERTICAL_ALIGNMENT_CENTER;
    txAttachmentInfos[1].sizeSetting            = TX_SIZE_CLIENT_RESOLUTION_DEPENDENT;

    pUIHandler->AttachTextures(m_PlayButtonEntity, txAttachmentInfos, panelTextures.data(), panelTextures.size());

    const UIButtonComponent buttonComponent = {
        .defaultHighlight = buttonHighlight,
        .hoverHighlight = { 0.1f, 0.0f, 0.0f, 1.0f },
        .pressHighlight = { 0.2f, 0.0f, 0.0f, 1.0f },
        .onPress = std::bind(&MainMenuState::createGameSession, this)
    };

    pECS->AddComponent(m_PlayButtonEntity, buttonComponent);
}

void MainMenuState::createGameSession()
{
    GameSession* pGameSession = DBG_NEW GameSession(this);
    m_pStateManager->EnqueueStateTransition(pGameSession, STATE_TRANSITION::POP_AND_PUSH);
}
