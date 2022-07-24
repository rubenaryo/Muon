/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2019/12
Description : Definitions for OO-Window Creation
----------------------------------------------*/
#ifndef APPWINDOW_H
#define APPWINDOW_H

#include "Game.h"

namespace System {
// A templated base class that other windows will inherit from
template <class WindowType>
class BaseWindow
{
public:
    // Static Window Procedure, who's functionality is inherited by all who inherit from this class
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        WindowType* pWindow = NULL;

        if (uMsg == WM_NCCREATE)
        {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pWindow = (WindowType*)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pWindow);

            pWindow->mHwnd = hwnd;
        }
        else
        {
            pWindow = (WindowType*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }

        // Callback to our own message handler
        if (pWindow)
            return pWindow->HandleMessage(uMsg, wParam, lParam);
        else
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    // Default constructor initializes to NULL
    BaseWindow() : mpGame(nullptr), mHwnd(nullptr) {};

    // Method to create and register a WNDCLASS
    BOOL Create(Core::Game* pGame, PCWSTR lpWindowName, HINSTANCE hInstance, DWORD dwStyle, DWORD dwExStyle = 0, LONG x = 0, LONG y = 0, LONG nWidth = 800, LONG nHeight = 600, HWND hWndParent = 0, HMENU hMenu = 0);

    // Accessor for member HWND
    HWND Hwnd() const { return mHwnd; }

protected:
    Core::Game* mpGame;

    // Handle to created window
    HWND mHwnd;

    // Pure virtual functions that must be derived by any windows
    virtual PCWSTR ClassName() const = 0;
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
    virtual bool InitGame(HWND hwnd, int width, int height) = 0;
};

// A derived GameWindow class
class GameWindow : public BaseWindow<GameWindow>
{
public:
    // Default constructor just initializes Game pointer
    GameWindow() {};

    // Overriden pure virtual functions from BaseWindow
    PCWSTR ClassName() const final { return L"Game Window Class"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) final;
    bool InitGame(HWND hwnd, int width, int height) override final;
    void RunGame();
        
private:
    // States of the window (used to send messages to DirectX on resize, move, etc)
    bool mResizeMove = false;
    bool mSuspended  = false;
    bool mMinimized  = false;
    bool mFullScreen = false;
};

// Definition of templated Create Method
template<class WindowType>
BOOL BaseWindow<WindowType>::Create(Core::Game* pGame, PCWSTR lpWindowName, HINSTANCE hInstance, DWORD dwStyle, DWORD dwExStyle, LONG x, LONG y, LONG nWidth, LONG nHeight, HWND hWndParent, HMENU hMenu)
{
    WNDCLASSEXW wc = {0};

    // Define WNDCLASS fields
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = dwStyle;
    wc.lpfnWndProc = WindowType::WindowProc;
    wc.hInstance = hInstance;
    //wc.hIcon = LoadIconW(hInstance, L"IDI_ICON");
    //wc.hIconSm = LoadIconW(wc.hInstance, L"IDI_ICON");
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW); // TODO: Import custom cursor
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = ClassName();

    if (!RegisterClassExW(&wc))
        return FALSE;
    
    RECT rc = { x, y, x + nWidth, y + nHeight};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    mHwnd = CreateWindowExW(dwExStyle, ClassName(), lpWindowName, dwStyle, x, y, rc.right - rc.left, rc.bottom - rc.top, hWndParent, hMenu, hInstance, this);

    // Initialize Windows Imaging Component (WIC)
    if (FAILED(CoInitializeEx(NULL, COINITBASE_MULTITHREADED)))
        return FALSE;

    if (mHwnd)
    {
        mpGame = pGame;
        return InitGame(mHwnd, nWidth, nHeight);
    }
    else
    {
        return FALSE;
    }
}

}
#endif