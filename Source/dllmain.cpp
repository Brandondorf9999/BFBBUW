#include "../Source/pch.h"
#include <Windows.h>
#include <iostream>

using namespace std;

DWORD fovAddress = 0x2CD03B0;
DWORD fpsAddress = 0x04593398;

void StartPatch()
{
    #if defined _DEBUG // Checks if the project build is Debug release.
        AllocConsole(); // Adds console window for testing purposes
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout); // Allows us to add outputs to the ASI Loader Console Window.
        cout << "21xMachi9 Loaded!" << endl; // Tells us that the ASI Plugin loaded successfully.
    #endif // Hides the debug window on Release builds.
    //Sleep(2000);
    *(float*)fovAddress = 0.0116563337; // Changes vertical field of view.
    *(float*)fpsAddress = 9999; // Changes Max Framerate.
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) // This code runs when the DLL file has been attached to the game process.
    {
        StartPatch(); // Calls the StartPatch function on start.
    }
    return TRUE;
}