#pragma once

#ifdef TOUCAN_DEBUG
    #ifdef TOUCAN_PLATFORM_WINDOWS
        #include <crtdbg.h>
        #include <iostream>

        #define DBG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__)
    #else
        #define DBG_NEW new
    #endif

#else
    #define DBG_NEW new
#endif
