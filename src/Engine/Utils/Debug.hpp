#pragma once

#ifdef _DEBUG
    #ifdef _MSC_VER
        #include <crtdbg.h>
        #include <iostream>

        #define _CRTDBG_MAP_ALLOC
        #define DBG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__)

    #else
        #define DBG_NEW new
    #endif

#else
    #define DBG_NEW new
#endif
