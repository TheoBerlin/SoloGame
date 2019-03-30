#define CATCH_CONFIG_RUNNER

#include <Engine/Rendering/Display.hpp>
#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>
#include <Engine/Rendering/AssetLoaders/TextureLoader.hpp>
#include <Engine/Utils/Logger.hpp>
#include <catch/catch.hpp>
#include <crtdbg.h>

int main(int argc, char* argv[])
{
	// Global setup
	Logger::init();
	Display display;

	if (!display.init(720, 16.0f/9.0f, true)) {
		return 1;
	}

	TextureLoader::setDevice(display.getDevice());

	// Check for memory leaks. Disabled when not debugging
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // Run tests
	int result = Catch::Session().run(argc, argv);

	// Global tear-down
	ModelLoader::deleteAllModels();
    TextureLoader::deleteAllTextures();

	// Keep console open
	std::getchar();

	return result;
}
