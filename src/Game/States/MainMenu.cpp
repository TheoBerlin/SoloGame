#include "MainMenu.hpp"

#include <Engine/Audio/SoundHandler.hpp>
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

MainMenu::MainMenu(StateManager* pStateManager, ECSCore* pECS, Device* pDevice, InputHandler* pInputHandler)
    :State(pStateManager, pECS),
    m_pDevice(pDevice),
    m_pInputHandler(pInputHandler)
{}

void MainMenu::init()
{
    EntityPublisher* pEntityPublisher = m_pECS->getEntityPublisher();

    m_pInputHandler->showCursor();

    UIHandler* pUIHandler       = reinterpret_cast<UIHandler*>(pEntityPublisher->getComponentHandler(TID(UIHandler)));
    TextRenderer* pTextRenderer = reinterpret_cast<TextRenderer*>(pEntityPublisher->getComponentHandler(TID(TextRenderer)));
    TextureCache* pTextureCache = reinterpret_cast<TextureCache*>(pEntityPublisher->getComponentHandler(TID(TextureCache)));

    // Create UI panel
    uiEntity = m_pECS->createEntity();
    pUIHandler->createPanel(uiEntity, {0.4f, 0.45f}, {0.2f, 0.1f}, {0.0f, 0.0f, 0.0f, 0.0f}, 1.0f);

    // Attach background and text textures to the panel
    std::array<std::shared_ptr<Texture>, 2> panelTextures = {
        pTextureCache->loadTexture("./assets/Models/Cube.png"),
        pTextRenderer->renderText("Play", "assets/Fonts/arial/arial.ttf", 50)
    };

    TextureAttachmentInfo txAttachmentInfos[2];
    txAttachmentInfos[0].sizeSetting            = TX_SIZE_STRETCH;
    txAttachmentInfos[1].horizontalAlignment    = TX_HORIZONTAL_ALIGNMENT_CENTER;
    txAttachmentInfos[1].verticalAlignment      = TX_VERTICAL_ALIGNMENT_CENTER;
    txAttachmentInfos[1].sizeSetting            = TX_SIZE_CLIENT_RESOLUTION_DEPENDENT;

    pUIHandler->attachTextures(uiEntity, txAttachmentInfos, panelTextures.data(), panelTextures.size());

    // Make the panel a button
    pUIHandler->createButton(uiEntity, {0.1f, 0.0f, 0.0f, 1.0f}, {0.2f, 0.0f, 0.0f, 1.0f}, std::bind(&MainMenu::createGameSession, this));

    // Create test sound
    Entity soundEntity = m_pECS->createEntity();
    const std::string soundFile = "./assets/Sounds/muscle-car-daniel_simon.mp3";

    SoundHandler* pSoundHandler = reinterpret_cast<SoundHandler*>(pEntityPublisher->getComponentHandler(TID(SoundHandler)));
    if (pSoundHandler->createSound(soundEntity, soundFile)) {
        pSoundHandler->playSound(soundEntity);
    }

    LOG_INFO("Entered main menu, press E to start a game session");
}

void MainMenu::resume()
{
    m_pInputHandler->showCursor();
}

void MainMenu::pause()
{}

void MainMenu::update(float dt)
{
    UNREFERENCED_VARIABLE(dt);

    if (m_pInputHandler->keyState(GLFW_KEY_E)) {
        createGameSession();
    }
}

void MainMenu::createGameSession()
{
    GameSession* pGameSession = DBG_NEW GameSession(this);
    m_pStateManager->enqueueStateTransition(pGameSession, STATE_TRANSITION::POP_AND_PUSH);
}
