#include <Engine/Utils/Debug.hpp>
#include <Engine/Utils/Logger.hpp>
#include <Engine/Utils/ThreadPool.hpp>
#include <Game/Game.hpp>

#include <argh/argh.h>

int main(int argc, char** argv)
{
    // Check for memory leaks. Disabled when not debugging
    #ifdef CONFIG_DEBUG
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    #endif

    Logger::init();
    argh::parser flagParser(argc, argv);

    ThreadPool::GetInstance().Init();

    Game game;
    if (!game.Init() || !game.Finalize(flagParser)) {
        return 1;
    }

    game.Run();

    return 0;
}
