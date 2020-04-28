// dllmain.cpp: This is where the magic happens!

/**
 21xMachi9 is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 21xMachi9 is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
**/

#define _CRT_SECURE_NO_WARNINGS

#include "../Source/pch.h"
#include "wtypes.h"
#include "../Source/ThirdParty/inipp/inipp/inipp.h"
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <math.h>

int tMaxFPS;
float FOV;
int consoleWindow;
float pi = 3.14159265358979323846;
bool debugBuild = false;

static void UnprotectModule(HMODULE p_Module)
{
    //This function was provided by Orfeasz
    PIMAGE_DOS_HEADER s_Header = (PIMAGE_DOS_HEADER)p_Module;
    PIMAGE_NT_HEADERS s_NTHeader = (PIMAGE_NT_HEADERS)((DWORD)p_Module + s_Header->e_lfanew);

    SIZE_T s_ImageSize = s_NTHeader->OptionalHeader.SizeOfImage;

    DWORD s_OldProtect;
    VirtualProtect((LPVOID)p_Module, s_ImageSize, PAGE_EXECUTE_READWRITE, &s_OldProtect);
}

void createConsole()
{
    AllocConsole(); // Adds console window for testing purposes.
    freopen("CONOUT$", "w", stdout); // Allows us to add outputs to the ASI Loader Console Window.
    std::cout.clear();
    std::cin.clear();
    std::cout << "21xMachi9 Loaded!" << std::endl; // Tells us that the ASI Plugin loaded successfully.
}

void parseIni() //Parses settings from the config.ini file.
{
    inipp::Ini<char> config; // Creates Inipp reference.
    std::ifstream is("config.ini"); // Checks for config.ini.
    config.parse(is); // if so, the "config.ini" will be parsed.
    config.generate(std::cout);
    config.default_section(config.sections["Settings"]);
    config.interpolate();
    if (!debugBuild) // Checks if project build is Release, since creating another consoleWindow in Debug is redundant.
    {
        inipp::extract(config.sections["DebugSettings"]["ConsoleWindow"], consoleWindow);
        if (consoleWindow == 1)
        {
            createConsole();
        }
    }
    inipp::extract(config.sections["Settings"]["t.MaxFPS"], tMaxFPS); // Grabs "t.MaxFPS" from config.ini.
}

// Get the horizontal and vertical screen sizes in pixel
void GetDesktopResolution(int& horizontal, int& vertical)
{
    RECT desktop;
    // Get a handle to the desktop window
    const HWND hDesktop = GetDesktopWindow();
    // Get the size of screen to the variable desktop
    GetWindowRect(hDesktop, &desktop);
    // The top left corner will have coordinates (0,0)
    // and the bottom right corner will have coordinates
    // (horizontal, vertical)
    horizontal = desktop.right;
    vertical = desktop.bottom;
}

void FOVCalc()
{
    // Declare the Vertical and Horizoontal Resolution variables.
    int hRes = 0;
    int vRes = 0;

    // Declares the original 16:9 Vert- FOV.
    float originalFOV = 0.008726646192;
    float originalAspectRatio = 1.777777777777778;

    // Checks for GameUserSettings.ini.
    GetFileAttributes("%LOCALAPPDATA%\\DXM\\Saved\\Config\\WindowsNoEditor\\GameUserSettings.ini");
    if (INVALID_FILE_ATTRIBUTES == GetFileAttributes("%LOCALAPPDATA%\\DXM\\Saved\\Config\\WindowsNoEditor\\GameUserSettings.ini") && GetLastError() == ERROR_FILE_NOT_FOUND)
    {
        //GameUserSettings.ini not found.
        std::cout << "ERROR: GameUserSettings.ini not found! Falling back to check game process resolution." << std::endl;
        HWND hwndWindow = FindWindow(NULL, TEXT("DaemonXMachina")); // Check for main process window.
        if (hwndWindow != 0) // if the window exists.
        {
            std::cout << "Falling back to game process resolution." << std::endl;

            // Get Window Size.
            RECT gameWindow;
            GetWindowRect(hwndWindow, &gameWindow);
            hRes = gameWindow.right;
            vRes = gameWindow.bottom;
            std::cout << "Resolution:" << hRes << "x" << vRes << std::endl;
        }
        else
        {
            std::cout << "ERROR: game process resolution not found! Falling back to desktop resolution." << std::endl;

            // Get desktop resolution.
            GetDesktopResolution(hRes, vRes);
            std::cout << "Resolution:" << hRes << "x" << vRes << std::endl;
        }
    }
    else
    {
        // GameUserSettings.ini found.
        inipp::Ini<char> GameUserSettings; // Creates Inipp reference.
        std::ifstream is("%LOCALAPPDATA%\\DXM\\Saved\\Config\\WindowsNoEditor\\GameUserSettings.ini"); // Checks for GameUserSettings.ini.
        GameUserSettings.parse(is); // if so, the "GameUserSettings.ini" will be parsed.
        GameUserSettings.generate(std::cout);
        GameUserSettings.default_section(GameUserSettings.sections["/Script/Engine.GameUserSettings"]);
        GameUserSettings.interpolate();
        inipp::extract(GameUserSettings.sections["/Script/Engine.GameUserSettings"]["ResolutionSizeX"], hRes); // Grabs "ResolutionSizeX" from GameUserSettings.ini.
        inipp::extract(GameUserSettings.sections["/Script/Engine.GameUserSettings"]["ResolutionSizeY"], vRes); // Grabs "ResolutionSizeY" from GameUserSettings.ini.
        std::cout << "Resolution:" << hRes << "x" << vRes << std::endl;
    }
    // Convert the int values to floats, so then we can determine the aspect ratio.
    float AspectRatio = (float)hRes / (float)vRes;
    std::cout << "Aspect Ratio:" << AspectRatio << std::endl;
    // Calculates the Vertical Field of View
    float FOV = atan(tan(originalFOV * (float)pi / 360.0f) / (AspectRatio) * (originalAspectRatio)) * 360.0f / (float)pi;
    std::cout << "New FOV Value:" << FOV << std::endl;
}

void StartPatch()
{
#if defined _DEBUG // Checks if the project build is Debug release.
    {
        debugBuild = true;
        createConsole();
    }
#endif // Hides the debug window on Release builds, if ConsoleWindow isn't open in config.ini.

    parseIni();
    // Gets the base module address, and also unprotects it from write protection.
    HMODULE baseModule = GetModuleHandle(NULL);
    //UnprotectModule(baseModule);

    Sleep(10000); // Sleeps the thread for ten seconds before applying the memory values.
    FOVCalc(); // Calculates the New Vertical FOV.



    //Writes FPS Cap to Memory, alongside pointer.
    *(float*)(*((intptr_t*)((intptr_t)baseModule + 0x4593398)) + 0x0) = tMaxFPS;
    // Writes FOV to Memory.
    *(float*)((intptr_t)baseModule + 0x2CD03B0) = FOV;
    // Writes Pillarbox Removal into Memory ("33 83 4C 02" to "33 83 4C 00").
    *(BYTE*)((intptr_t)baseModule + 0x1E14850 + 0x3) = 00;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) // This code runs when the DLL file has been attached to the game process.
    {
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)StartPatch, NULL, NULL, NULL); // Calls the StartPatch function in a new thread on start.
    }
    return TRUE;
}