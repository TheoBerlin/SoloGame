#pragma once

#include <Engine/GameState/State.hpp>
#include <Game/Level/Tube.hpp>
#include <Game/LightSpinner.hpp>
#include <Game/Racer/Components/Track.hpp>
#include <Game/Racer/Systems/RacerController.hpp>

class InputHandler;
class MainMenu;
class ModelLoader;
class SoundHandler;

class GameSession : public State
{
public:
    GameSession(MainMenu* pMainMenu);
    ~GameSession() = default;

    void init() override final;

    void resume() override final;
    void pause() override final;

    void update(float dt);

private:
    void startMusic(SoundHandler* pSoundHandler);
    void createCube(const DirectX::XMFLOAT3& position, const std::string& soundPath, SoundHandler* pSoundHandler, TransformHandler* pTransformHandler, ModelLoader* pModelLoader);
    void createPointLights(SoundHandler* pSoundHandler, TransformHandler* pTransformHandler, ComponentSubscriber* pComponentSubscriber);
    void createTube(const std::vector<DirectX::XMFLOAT3>& sectionPoints, TransformHandler* pTransformHandler, ModelLoader* pModelLoader);
    void createPlayer(TransformHandler* pTransformHandler, ComponentSubscriber* pComponentSubscriber);

private:
    // Component handlers
    TrackHandler m_TrackPositionHandler;
    TubeHandler m_TubeHandler;

    // Systems
    LightSpinner m_LightSpinner;
    RacerController m_RacerMover;

    InputHandler* m_pInputHandler;
};
