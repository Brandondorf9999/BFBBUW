#include "../Source/pch.h"
#include <Windows.h>

using namespace std;

void StartPatch()
{
    *(float*)0x04593398 = 9999; // Changes Max Framerate
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) // This code runs when the DLL file has been attached to the game process.
    {
        StartPatch(); // Calls the StartPatch function on start.
    }
    return TRUE;
}