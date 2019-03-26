#define CATCH_CONFIG_RUNNER

#include <catch/catch.hpp>

int main(int argc, char* argv[])
{
    // Run tests
	int result = Catch::Session().run(argc, argv);

	// Keep console open
	std::getchar();

	return result;
}
