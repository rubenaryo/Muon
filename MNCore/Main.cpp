/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2019/10
Description : This file contains the main function (entry point) for the application
----------------------------------------------*/
#include "MNCore_pch.h"
#include "AppWindow.h"

#include "Game.h"

#if defined(MN_DEBUG)
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h> 
#include <dxgidebug.h>
#endif

void Run(_In_ HINSTANCE hInstance, _In_ int nCmdShow)
{
    using System::GameWindow;

    Core::Game game;

    // Create the window
    GameWindow window;
    DWORD style = CS_HREDRAW | CS_VREDRAW;
    DWORD ExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE | WS_EX_OVERLAPPEDWINDOW;
    if (!window.Create(&game, L"Muon", hInstance, style, ExStyle, 0L, 0L, 1280L, 800L, 0, 0))
        exit(EXIT_FAILURE);

    // Show the window
    ShowWindow(window.Hwnd(), nCmdShow);

    // Run the game
    window.RunGame();

    DestroyWindow(window.Hwnd());
}

// Entry point for the application
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    using System::GameWindow;

    // On Debug Builds: Enable simple runtime memory check
    #if defined(MN_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    #endif 

    Run(hInstance, nCmdShow);

    // Dump any found memory leaks
    #if defined(MN_DEBUG)
    _CrtDumpMemoryLeaks();
    #endif

    exit(EXIT_SUCCESS);
}