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

#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS

#include "../Source/pch.h"
#include "../Source/ThirdParty/inipp/inipp/inipp.h"
#include <Windows.h>
#include <iostream>
#include <fstream>

int tMaxFPS;
float FOV;
int consoleWindow;
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
    AllocConsole(); // Adds console window for testing purposes
    freopen("CONOUT$", "w", stdout); // Allows us to add outputs to the ASI Loader Console Window.
    std::cout.clear();
    std::cin.clear();
    std::cout << "21xMachi9 Loaded!" << std::endl; // Tells us that the ASI Plugin loaded successfully.
}

void parseIni() //Parses settings from the config.ini file
{
    inipp::Ini<char> config; // Creates Inipp reference
    std::ifstream is("config.ini"); // Checks for config.ini
    config.parse(is); // if so, the config.ini will be parsed.
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
    inipp::extract(config.sections["Settings"]["t.MaxFPS"], tMaxFPS); // Grabs t.MaxFPS from config.ini

    inipp::Ini<char> GUSIni;
    char* localappdata = getenv("LOCALAPPDATA");

}

void FOVCalc()
{
    // Declare the Vertical and Horizoontal Resolution variables
    int hRes = 0;
    int vRes = 0;

    // Declares the original 16:9 Vert- FOV
    float originalFOV = 0.008726646192;
    float originalAspectRatio = 1.777777777777778;

    // If GameUserSettings.ini exists, grab in-game resolution from that
    {

    }
    // Otherwise, get desktop resolution
    {
        
    }
    // Convert the int values to floats, so then we can determine the aspect ratio
    float AspectRatio = (float)hRes / (float)vRes;

    //Arctan(Tan(originalFOV * (float)M_PI / 360.0f) / (AspectRatio) * (originalAspectRatio)) * 360.0f / (float)M_PI
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
    UnprotectModule(baseModule);

    Sleep(10000); // Sleeps the thread for ten seconds before applying the memory values.
    //Writes FPS Cap to Memory
    *(float*)(*((intptr_t*)((intptr_t)baseModule + 0x4593398)) + 0x0) = tMaxFPS;
    // Writes FOV to Memory
    *(float*)((intptr_t)baseModule + 0x2CD03B0) = 0.0116563337; // Test change to 21:9 Vert- FOV, will implement proper FOV scaling later
    // Writes Pillarbox Removal into Memory ("33 83 4C 02" to "33 83 4C 00")
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