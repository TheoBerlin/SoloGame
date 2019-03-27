#define CATCH_CONFIG_RUNNER

#include <catch/catch.hpp>
#include <crtdbg.h>

int main(int argc, char* argv[])
{
	// Global setup
	// Check for memory leaks. Disabled when not debugging
	_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF);

    // Run tests
	int result = Catch::Session().run(argc, argv);

	// Keep console open
	std::getchar();

	return result;
}
