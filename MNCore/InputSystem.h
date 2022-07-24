/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2019/10
Description : Interface for the InputSystem class
----------------------------------------------*/
#ifndef INPUTSYSTEM_H
#define INPUTSYSTEM_H

#include "MNCore_pch.h"
#include <array>
#include <unordered_map>
#include "InputBinding.h"

namespace Core 
{
    class InputSystem
    {
    public:
        InputSystem();
        virtual ~InputSystem();

        void Shutdown();
    
        // Main "Update method" for input system
        void GetInput();
    
        // On WM_MOUSEMOVE message, trigger this method
        void OnMouseMove(short newX, short newY);
    
        // Returns the current mouse position as a POINT
        POINT GetMousePosition() const { return mMouseCurrent; }
    
        // Returns the difference between current and previous as a std::pair
        std::pair<float, float> GetMouseDelta() const;
    private:
        // Uses GetAsyncKeyState to read in 256 bytes
        void GetKeyboardState();
    
        // returns the state of the key in enum form
        const KeyState GetKeyboardKeyState(const unsigned int keyCode) const;
    
        // returns true if the key is down
        inline const bool isPressed(int keyCode) const
        {
            return (GetAsyncKeyState(keyCode) & 0x8000) ? 1 : 0;
        }
    
        void Update();

        // Keyboard States
        std::array<BYTE, 256> mKeyboardCurrent;
        std::array<BYTE, 256> mKeyboardPrevious;
    
        // Mouse States
        POINT mMouseCurrent;
        POINT mMousePrevious;

    protected:
        std::unordered_map<GameCommands, Chord*> mKeyMap;
        std::unordered_map<GameCommands, Chord*> mActiveKeyMap;
    
        virtual void SetDefaultKeyMap() = 0;
    };

}
#endif