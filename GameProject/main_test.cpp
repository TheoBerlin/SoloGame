#define CATCH_CONFIG_RUNNER

#include <Engine/Utils/Logger.hpp>
#include <catch/catch.hpp>
#include <crtdbg.h>

#include <ft2build.h>
#include FT_FREETYPE_H

int main(int argc, char* argv[])
{
	// Check for memory leaks. Disabled when not debugging
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// Global setup
	Logger::init();

	// Initialize FreeType library
	FT_Library ftLib;
	FT_Error err = FT_Init_FreeType(&ftLib);
	if (!err) {
		Logger::LOG_INFO("Initialized FreeType library");
	} else {
		Logger::LOG_ERROR("Failed to initialize FreeType library: %s", FT_Error_String(err));
	}

    // Run tests
	int result = Catch::Session().run(argc, argv);

	// Global tear-down
	FT_Done_FreeType(ftLib);

	// Keep console open
	std::getchar();

	return result;
}
