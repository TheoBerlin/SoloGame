#define NOMINMAX
#include <Engine/Utils/Logger.hpp>
#include <Game/Game.hpp>
#include <crtdbg.h>

#include <stdio.h>
#include <io.h>
#include <windows.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	// Check for memory leaks. Disabled when not debugging
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Logger::init();

	Game game(hInstance);
    game.run();

	return 0;
}
