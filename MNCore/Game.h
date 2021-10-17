/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2019/12
Description : Interface for central game class
This class encapsulates all app functionality
----------------------------------------------*/
#ifndef GAME_H
#define GAME_H

#include "StepTimer.h"
#include "GameInput.h"

namespace Core {

class alignas(8) Game final
{
friend class AppWindow;

public:
    Game();
    ~Game();
    bool Init(HWND window, int width, int height);

    // Main Game Loop
    void Frame();

    // Callbacks for windows messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnMove();
    void OnResize(int newWidth, int newHeight);

    // Input Callbacks
    void OnMouseMove(short newX, short newY);

    // Cleanup
    void Shutdown();

private:
    void Update(StepTimer const& timer);
    void Render();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources(int newWidth, int newHeight);

    // Input Management
    GameInput mInput;
    
    // Timer for the main game loop
    StepTimer mTimer;

    HWND mHwnd;
};

}
#endif