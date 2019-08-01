#define NOMINMAX
#include <Engine/Utils/Logger.hpp>
#include <Game/Game.hpp>
#include <crtdbg.h>

int main()
{
	// Check for memory leaks. Disabled when not debugging
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Logger::init();

	Game game;

	return 0;
}
