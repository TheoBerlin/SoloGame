#include "IGame.hpp"

IGame::IGame(HINSTANCE hInstance)
    :display(hInstance, 720, 16.0f/9.0f, true),
    inputHandler(&ecs.systemSubscriber, display.getWindow()),
    transformHandler(&ecs.systemSubscriber),
    shaderHandler(display.getDevice(), &ecs.systemSubscriber),
    shaderResourceHandler(&ecs.systemSubscriber, display.getDevice()),
    txLoader(&ecs.systemSubscriber, display.getDevice()),
    modelLoader(&ecs.systemSubscriber, &txLoader),
    renderableHandler(&ecs.systemSubscriber),
    vpHandler(&ecs.systemSubscriber),
    lightHandler(&ecs.systemSubscriber),
    uiHandler(&ecs.systemSubscriber),
    renderer(&ecs, display.getDevice(), display.getDeviceContext(), display.getRenderTarget(), display.getDepthStencilView()),
    uiRenderer(&ecs, display.getDeviceContext(), display.getDevice(), display.getRenderTarget(), display.getDepthStencilView()),
    cameraSystem(&ecs),
    buttonSystem(&ecs, display.getWindowWidth(), display.getWindowHeight())
{
    display.showWindow();
    renderer.update(0.0f);
}

IGame::~IGame()
{}

void IGame::run()
{
    auto timer = std::chrono::high_resolution_clock::now();
    auto timeNow = timer;
    std::chrono::duration<float> dtChrono;

    MSG msg = {0};
    while(WM_QUIT != msg.message)
    {
        if (!Display::keepRunning)
        {
            break;
        }
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        } else {
            timeNow = std::chrono::high_resolution_clock::now();
            dtChrono = timeNow-timer;
            float dt = dtChrono.count();

            timer = timeNow;

            inputHandler.update();

            // Update logic
            stateManager.update(dt);

            buttonSystem.update(dt);

            // Render
            display.clearBackBuffer();
            renderer.update(dt);
            uiRenderer.update(dt);
            display.presentBackBuffer();

            // Perform maintenance
            const std::vector<Entity>& entitiesToDelete = ecs.systemSubscriber.getEntitiesToDelete();
            if (!entitiesToDelete.empty()) {
                ecs.systemSubscriber.performDeletions();

                for (Entity deletedEntity : entitiesToDelete) {
                    ecs.entityIDGen.popID(deletedEntity);
                }
            }
        }
    }
}
