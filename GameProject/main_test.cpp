#define CATCH_CONFIG_RUNNER

#include <Engine/Utils/Logger.hpp>
#include <catch/catch.hpp>
#include <crtdbg.h>

int main(int argc, char* argv[])
{
	// Global setup
	Logger::init();

	// Check for memory leaks. Disabled when not debugging
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // Run tests
	int result = Catch::Session().run(argc, argv);

	// Global tear-down

	// Keep console open
	std::getchar();

	return result;
}
