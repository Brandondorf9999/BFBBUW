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

#include "../Source/pch.h"
#include <Windows.h>
#include <iostream>
#include <fstream>
#include "../Source/ThirdParty/inipp/inipp/inipp.h"

using namespace std;

DWORD fovAddress = 0x2CD03B0;
DWORD fpsAddress = 0x04593398;
int tMaxFPS;
int consoleWindow;
bool debugBuild = false;

void createConsole()
{
    AllocConsole(); // Adds console window for testing purposes
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout); // Allows us to add outputs to the ASI Loader Console Window.
    cout << "21xMachi9 Loaded!" << endl; // Tells us that the ASI Plugin loaded successfully.
}

void parseIni() //Parses settings from the config.ini file
{
    inipp::Ini<char> config; // Creates Inipp reference
    std::ifstream is("config.ini"); // Checks for config.ini
    config.parse(is); // if so, the config.ini will be parsed.
    config.generate(std::cout);
    config.default_section(config.sections["Settings"]);
    config.generate(std::cout);
    if (!debugBuild) // Checks if project build is Release, since creating another consoleWindow in Debug is redundant.
    {
        inipp::extract(config.sections["DebugSettings"]["ConsoleWindow"], consoleWindow);
        if (consoleWindow = 1)
        {
            createConsole();
        }
    }
    inipp::extract(config.sections["Settings"]["t.MaxFPS"], tMaxFPS); // Grabs t.MaxFPS from config.ini
    cout << "tMaxFPS = " << tMaxFPS << endl; // Displays the ini's current framerate value
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
    //Sleep(2000);
    //*(float*)fovAddress = 0.0116563337; // Changes vertical field of view.
    //*(float*)fpsAddress = tMaxFPS; // Changes Max Framerate.
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) // This code runs when the DLL file has been attached to the game process.
    {
        StartPatch(); // Calls the StartPatch function on start.
    }
    return TRUE;
}