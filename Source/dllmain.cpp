// dllmain.cpp: This is where the magic happens!

/**
 21xMachi9 (C) 2020 Bryce Q.

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
#define _USE_MATH_DEFINES

#include "../Source/pch.h"
#include "wtypes.h"
#include "../Source/ThirdParty/inipp/inipp/inipp.h"
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include "../Source/ThirdParty/ModUtils/MemoryMgr.h"

using namespace std;

bool debugMode;
float FOV;
int tMaxFPS;


void createConsole()
{
    AllocConsole(); // Adds console window for testing purposes.
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout); // Allows us to add outputs to the ASI Loader Console Window.
    cout.clear();
    cin.clear();
    cout << "21xMachi9 Loaded!" << endl; // Tells us that the ASI Plugin loaded successfully.
}

void parseIni() //Parses settings from the config.ini file.
{
	// Gets the baseModule and then unprotects it (if needed).
	HMODULE baseModule = GetModuleHandle(NULL);
	ScopedUnprotect::FullModule UnProtect(baseModule);;

    inipp::Ini<char> config; // Creates Inipp reference.
    ifstream is("config.ini"); // Checks for config.ini.
    config.parse(is); // if so, the "config.ini" will be parsed.
    config.generate(cout);
    config.default_section(config.sections["Settings"]);
    config.interpolate();
    inipp::extract(config.sections["Settings"]["t.MaxFPS"], tMaxFPS); // Grabs "t.MaxFPS" from config.ini.

	// Shows debug logs in debug mode.
	if (debugMode)
	{
		cout << "Current t.MaxFPS in config.ini: " << tMaxFPS << endl;
	}

    //Writes FPS Cap to Memory, alongside pointer.
    *(float*)(*((intptr_t*)((intptr_t)baseModule + 0x4593398)) + 0x0) = (float)tMaxFPS;
}

void fovCalc()
{
	// Gets the baseModule and then unprotects it (if needed).
	HMODULE baseModule = GetModuleHandle(NULL);
	ScopedUnprotect::FullModule UnProtect(baseModule);;

    // Declare the Vertical and Horizontal Resolution variables.
    int hRes = *(int*)((intptr_t)baseModule + 0x416B840); // Grabs Horizontal Resolution integer
    int vRes = *(int*)((intptr_t)baseModule + 0x416B844); // Grabs Vertical Resolution integer

    // Declares the original 16:9 Vertical FOV.
    float originalFOV = 0.008726646192;
    float originalAspectRatio = 1.777777777777778;

    // Convert the int values to floats, so then we can determine the aspect ratio.
    float AspectRatio = (float)hRes / (float)vRes;
 
    // Calculates the Vertical Field of View using the new aspect ratio, the old aspect ratio, and the original FOV
    float FOV = std::round((2.0f * atan(((AspectRatio) / (16.0f / 9.0f)) * tan((originalFOV * 10000.0f) / 2.0f * ((float)M_PI / 180.0f)))) * (180.0f / (float)M_PI) * 100.0f) / 100.0f / 10000.0f;

    // Shows debug logs in debug mode.
    if (debugMode)
    {
		cout << "Resolution:" << hRes << "x" << vRes << endl;
		cout << "Aspect Ratio:" << AspectRatio << endl;
		cout << "New FOV Value:" << FOV << endl;
    }

    // Writes FOV to Memory.
    *(float*)((intptr_t)baseModule + 0x2CD03B0) = (float)FOV;
}

void pillarboxRemoval()
{
	// Gets the baseModule and then unprotects it (if needed).
	HMODULE baseModule = GetModuleHandle(NULL);
	ScopedUnprotect::FullModule UnProtect(baseModule);;

	// Writes Pillarbox Removal into Memory ("33 83 4C 02" to "33 83 4C 00").
	*(BYTE*)(*((intptr_t*)((intptr_t)baseModule + 0x1E14850)) + 0x3) = 00;
}

void StartPatch()
{
#if defined _DEBUG // Checks if build is Debug, and if so, creates a console and enables debugMode.
    {
        createConsole();
        debugMode = true;
    }
#endif

    Sleep(5000); // Sleeps the thread for five seconds before applying the memory values.

    parseIni(); // Reads from the config.ini file, and applies the new tMaxFPS value.

    fovCalc(); // Calculates the New Vertical FOV.

    //pillarboxRemoval(); // Removes the in-game pillarboxing.
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) // This code runs when the DLL file has been attached to the game process.
    {
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)StartPatch, NULL, NULL, NULL); // Calls the StartPatch function in a new thread on start.
    }
    return TRUE;
}