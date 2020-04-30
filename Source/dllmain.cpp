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
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include "../Source/ThirdParty/inipp/inipp/inipp.h"
#include "../Source/ThirdParty/ModUtils/MemoryMgr.h"

using namespace std;

// Config.ini variables
int useCustomFPSCap;
int useCustomFOV;
int tMaxFPS;
int ignoreUpdates;

// Resolution related variables
int hRes;
int vRes;
float originalAspectRatio = 1.777777777777778;
float aspectRatio;

// FOV variables
float originalFOV = 0.008726646192; // Declares the original 16:9 vertical FOV.
float customFOV;
float FOV;

// Misc variables
bool check = true; // do not change to false or else resolution checks won't run.
bool displayedStartupSettings = false; // Prevents the debug strings from being shown constantly on each update.
bool debugMode; // Allows for printing debug strings.


// Process HMODULE variable
HMODULE baseModule = GetModuleHandle(NULL);

void createConsole()
{
    AllocConsole(); // Adds console window for testing purposes.
    cout << "21xMachi9 Loaded!" << endl; // Tells us that the ASI Plugin loaded successfully.
}

void uncapFPS() //Uncaps the framerate.
{
    //Writes the new t.MaxFPS cap to memory, alongside pointer.
    *(float*)(*((intptr_t*)((intptr_t)baseModule + 0x4593398)) + 0x0) = (float)tMaxFPS;
}

void fovCalc()
{
    // Declare the vertical and horizontal resolution variables.
    int hRes = *(int*)((intptr_t)baseModule + 0x416B840); // Grabs Horizontal Resolution integer.
    int vRes = *(int*)((intptr_t)baseModule + 0x416B844); // Grabs Vertical Resolution integer.

    // Convert the int values to floats, so then we can determine the aspect ratio.
    float aspectRatio = (float)hRes / (float)vRes;

    if (useCustomFOV == 1)
    {
		// Calculates the vertical FOV using the new aspect ratio, the old aspect ratio, and the desired custom FOV.
        float FOV = std::round((2.0f * atan(((aspectRatio) / (16.0f / 9.0f)) * tan((customFOV * 10000.0f) / 2.0f * ((float)M_PI / 180.0f)))) * (180.0f / (float)M_PI) * 100.0f) / 100.0f / 10000.0f;
    }
    else
    {
		// Calculates the vertical FOV using the new aspect ratio, the old aspect ratio, and the original FOV.
		float FOV = std::round((2.0f * atan(((aspectRatio) / (16.0f / 9.0f)) * tan((originalFOV * 10000.0f) / 2.0f * ((float)M_PI / 180.0f)))) * (180.0f / (float)M_PI) * 100.0f) / 100.0f / 10000.0f;
    }
    
    // Shows debug logs in debug mode.
    if (debugMode)
    {
        if (displayedStartupSettings)
        {
			cout << "Startup Settings" << endl;
			cout << "Resolution:" << hRes << "x" << vRes << endl;
			cout << "Aspect Ratio:" << aspectRatio << endl;
			cout << "New FOV Value:" << FOV << endl;
            displayedStartupSettings = true;
        }
    }

    // Writes FOV to Memory.
    *(float*)((intptr_t)baseModule + 0x2CD03B0) = (float)FOV;
}

void resolutionCheck()
{
    if (aspectRatio != (*(int*)((intptr_t)baseModule + 0x416B840) / *(int*)((intptr_t)baseModule + 0x416B844)))
    {
        fovCalc();
    }
}

void framerateCheck()
{
	if (tMaxFPS != *(float*)(*((intptr_t*)((intptr_t)baseModule + 0x4593398)) + 0x0))
	{
		uncapFPS();
	}
}

void pillarboxRemoval()
{
    // Writes pillarbox removal into memory ("33 83 4C 02" to "33 83 4C 00").
	memcpy((LPVOID)((intptr_t)baseModule + 0x1E14850), "\x33\x83\x4c\x00", 4);
}

void readConfig()
{
    int customFOVConfig;
	inipp::Ini<char> config;
	ifstream iniName("config.ini");
    config.parse(iniName);
	config.generate(cout);
	config.default_section(config.sections["Settings"]);
    config.interpolate();
    config.generate(cout);
    inipp::extract(config.sections["FieldOfView"]["useCustomFOV"], useCustomFOV);
    config.generate(cout);
    inipp::extract(config.sections["FieldOfView"]["FOV"], customFOVConfig);
    config.generate(cout);
    inipp::extract(config.sections["Experimental"]["useCustomFPSCap"], useCustomFPSCap);
    config.generate(cout);
    inipp::extract(config.sections["Experimental"]["maxFPS"], tMaxFPS);
	config.generate(cout);
	inipp::extract(config.sections["Launcher"]["ignoreUpdates"], ignoreUpdates);

    customFOV = (float)customFOVConfig / 10000.0f;
}

void StartPatch()
{
    cout.flush();
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout); // Allows us to add outputs to the ASI Loader Console Window.
	cout.clear();
	cin.clear();
#if defined _DEBUG // Checks if build is Debug, and if so, creates a console and enables debugMode.
    {
        createConsole();
        debugMode = true;
    }
#endif

    // Reads the "config.ini" config file for values that we are going to want to modify.
    readConfig();

	// Unprotects the main module handle.
	ScopedUnprotect::FullModule UnProtect(baseModule);;

    Sleep(5000); // Sleeps the thread for five seconds before applying the memory values.

	fovCalc(); // Calculates the new vertical FOV.

	pillarboxRemoval(); // Removes the in-game pillarboxing.

    // checks if CustomFPS cap is enabled before choosing which processes to loop, and if to uncap the framerate. This is done to save on CPU resources.
    if (useCustomFPSCap == 1)
    {
        uncapFPS(); //Uncaps the framerate.
        // Runs resolution and framerate check in a loop.
		while (check != false)
		{
			resolutionCheck();
			framerateCheck();
		}
    }
    else
    {
        // Runs resolution check in a loop.
		while (check != false)
		{
			resolutionCheck();
		}
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) // This code runs when the DLL file has been attached to the game process.
    {
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)StartPatch, NULL, NULL, NULL); // Calls the StartPatch function in a new thread on start.
    }
    return TRUE;
}