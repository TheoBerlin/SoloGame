#define NOMINMAX

#include <Engine/Utils/Debug.hpp>
#include <Engine/Utils/Logger.hpp>
#include <Engine/Utils/ThreadPool.hpp>
#include <Game/Game.hpp>

#include <argh/argh.h>

#ifdef _DEBUG
    #include <crtdbg.h>
#endif

int main(int argc, char** argv)
{
    // Check for memory leaks. Disabled when not debugging
    #ifdef _DEBUG
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    #endif

    Logger::init();
    argh::parser flagParser(argc, argv);

    ThreadPool::getInstance().initialize();

    Game game;
    if (!game.init() || !game.finalize(flagParser)) {
        return 1;
    }

    game.run();

    return 0;
}
