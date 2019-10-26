#define NOMINMAX
#include <Engine/Utils/Logger.hpp>
#include <Game/Game.hpp>
#include <crtdbg.h>

#include <stdio.h>
#include <io.h>
#include <windows.h>

#include <ft2build.h>
#include FT_FREETYPE_H

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	// Check for memory leaks. Disabled when not debugging
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Logger::init();

	// Initialize FreeType library
	FT_Library ftLib;
	FT_Error err = FT_Init_FreeType(&ftLib);
	if (!err) {
		Logger::LOG_INFO("Initialized FreeType library");
	} else {
		Logger::LOG_ERROR("Failed to initialize FreeType library: %s", FT_Error_String(err));
	}

	Game game(hInstance);
    game.run();

	FT_Done_FreeType(ftLib);

	return 0;
}
