/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2019/12
Description : Implementation of Message Loop
----------------------------------------------*/
#include "MNCore_pch.h"
#include "AppWindow.h"

#include <exception>

namespace System {


LRESULT GameWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_MOUSEMOVE:
        POINTS pt = MAKEPOINTS(lParam);
        mpGame->OnMouseMove(pt.x, pt.y);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_PAINT:
    case WM_MOVE:
    case WM_SIZE:
        // If the window has just been minimized
        if (wParam == SIZE_MINIMIZED)
        {
            if (!mMinimized)
            {
                // Set minimized flag, suspend the game
                mMinimized = true;
                if (!mSuspended)
                    mpGame->OnSuspending();
                mSuspended = true;
            }
        }
        else if (mMinimized) // if we're minimized already, must be maximizing
        {
            // Set maximized flag, resume the game
            mMinimized = false;
            if (mSuspended)
                mpGame->OnResuming();
            mSuspended = false;
        }
        break;
    case WM_ENTERSIZEMOVE:
        mResizeMove = true;
        break;
    case WM_EXITSIZEMOVE:
    {
        mResizeMove = false;
        RECT rc;
        GetClientRect(mHwnd, &rc);
        mpGame->OnResize(static_cast<int>(rc.right - rc.left), static_cast<int>(rc.bottom - rc.top));  
    }
    break;
    case WM_GETMINMAXINFO:
    {
        // lParam represents default positions, dimensions, tracking sizes
        auto info = reinterpret_cast<MINMAXINFO*>(lParam);
        
        // Override these values so the client area doesn't get too small
        info->ptMinTrackSize.x = 320;
        info->ptMinTrackSize.y = 200;
    }
    break;
    case WM_ACTIVATEAPP:
        if (wParam)
            mpGame->OnActivated();
        else
            mpGame->OnDeactivated();
        break;
    case WM_POWERBROADCAST:
        switch (wParam)
        {
        case PBT_APMQUERYSUSPEND:
            if (!mSuspended)
                mpGame->OnSuspending();
            mSuspended = true;
            return TRUE;
        case PBT_APMRESUMESUSPEND:
            if (!mMinimized)
            {
                if (mSuspended)
                    mpGame->OnResuming();
                mSuspended = false;
            }
            return TRUE;
        }
        break;
    case WM_MENUCHAR:
        return MAKELRESULT(0, 1);
    default:
        return DefWindowProc(mHwnd, uMsg, wParam, lParam);
    }
    return TRUE;
}

bool GameWindow::InitGame(HWND hwnd, int width, int height)
{
#if defined(MN_DEBUG)
    bool result = false;
    try
    {
        result = mpGame->Init(hwnd, width, height);
    }
    catch (std::exception const& e)
    {
        MessageBoxA(hwnd, e.what(), "Fatal Exception!", MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
        DestroyWindow(hwnd);
    }
    return result;
#else
    return sGame->Init(hwnd, width, height);
#endif
}

// Here we invoke the window procedure to handle windows messages
// if no message is received, we process a frame
void GameWindow::RunGame()
{
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        // Process one gameplay frame
        mpGame->Frame();
    }
    mpGame->Shutdown();
}

}