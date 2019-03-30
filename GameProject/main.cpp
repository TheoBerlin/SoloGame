#include <Engine/Utils/Logger.hpp>
#include <crtdbg.h>

int main()
{
	// Check for memory leaks. Disabled when not debugging
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Logger::init();

	return 0;
}
