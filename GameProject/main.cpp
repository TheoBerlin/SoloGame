#define NOMINMAX

#include <Engine/Utils/Debug.hpp>
#include <Engine/Utils/Logger.hpp>
#include <Game/Game.hpp>

#ifdef _DEBUG
    #include <crtdbg.h>
#endif

int main()
{
    // Check for memory leaks. Disabled when not debugging
    #ifdef _DEBUG
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    #endif

    Logger::init();

    Game game;
    if (!game.init() || !game.finalize()) {
        return 1;
    }

    game.run();

    return 0;
}
