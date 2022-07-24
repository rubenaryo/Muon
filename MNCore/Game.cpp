/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2019/12
Description : Implementation of Game.h
----------------------------------------------*/
#include "MNCore_pch.h"
#include "Game.h"

#include "GameInput.h"

namespace Core {

// Initialize device resources, and link up this game to be notified of device updates
Game::Game()
    : mInput()
    , mTimer()
{
}

// Initialize device resource holder by creating all necessary resources
bool Game::Init(HWND hwnd, int width, int height)
{
    mHwnd = hwnd;

    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources(width, height);

    mInput.Init();
    mTimer.SetFixedTimeStep(false);

    return true;
}

// On Timer tick, run Update() on the game, then Render()
void Game::Frame()
{
    mTimer.Tick([&]()
    {
        Update(mTimer);
    });

    Render();
}

void Game::Update(StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // Update the input, passing in the camera so it will update its internal information
    mInput.Frame(elapsedTime, NULL);
}

void Game::Render()
{

}

void Game::CreateDeviceDependentResources()
{

}

void Game::CreateWindowSizeDependentResources(int newWidth, int newHeight)
{

}

void Game::Shutdown()
{
    mInput.Shutdown();
}

Game::~Game()
{
}

#pragma region Game State Callbacks

void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
}

void Game::OnResuming()
{
    mTimer.ResetElapsedTime();
}

// Recreates Window size dependent resources if needed
void Game::OnMove()
{
}

// Recreates Window size dependent resources if needed
void Game::OnResize(int newWidth, int newHeight)
{
#if defined(MN_DEBUG)
    try
    {
        CreateWindowSizeDependentResources(newWidth, newHeight);
    }
    catch (std::exception const& e)
    {
        MessageBoxA(mHwnd, e.what(), "Fatal Exception!", MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
        DestroyWindow(mHwnd);
    }
#else
    CreateWindowSizeDependentResources(newWidth, newHeight);
#endif
}

void Game::OnMouseMove(short newX, short newY)
{
    mInput.OnMouseMove(newX, newY);
}

#pragma endregion
}

