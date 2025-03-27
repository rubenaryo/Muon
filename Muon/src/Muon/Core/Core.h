/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2025/3
Description : Defines DLL import/export macro and other things
----------------------------------------------*/
#ifndef MUON_CORE_H
#define MUON_CORE_H

#if defined(MN_PLATFORM_WINDOWS)
    #if defined(MN_BUILD_DLL)
        #define MUON_API __declspec(dllexport)
    #else
        #define MUON_API __declspec(dllimport)
    #endif
#else
    #error Muon only currently support Windows!
#endif

#endif