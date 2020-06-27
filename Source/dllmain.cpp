// dllmain.cpp: This is where the magic happens!

/**
 BFBBUW (C) 2020 Bryce Q.

 BFBBUW is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 BFBBUW is distributed in the hope that it will be useful,
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
int customFOV;

// Resolution and Aspect Ratio variables
int hRes;
int vRes;
float originalAspectRatio = 1.777777777777778;
float aspectRatio;

// FOV variables
float originalFOV = 0.008726646192; // Declares the original 16:9 vertical FOV.
float FOV;

// Misc variables
bool check = true; // do not change to false or else resolution checks won't run.

// Process HMODULE variable
HMODULE baseModule = GetModuleHandle(NULL);

void readConfig()
{
	cout.flush();
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout); // Allows us to add outputs to the ASI Loader Console Window.
	cout.clear();
	cin.clear();
	inipp::Ini<char> config;
	ifstream configName("config.ini");
	config.parse(configName);
	config.generate(cout);
	config.default_section(config.sections["Settings"]);
	config.interpolate();
	inipp::extract(config.sections["FieldOfView"]["useCustomFOV"], useCustomFOV);
	inipp::extract(config.sections["FieldOfView"]["FOV"], customFOV);
	inipp::extract(config.sections["Experimental"]["useCustomFPSCap"], useCustomFPSCap);
	inipp::extract(config.sections["Experimental"]["maxFPS"], tMaxFPS);
	inipp::extract(config.sections["Launcher"]["ignoreUpdates"], ignoreUpdates);
}

void fovCalc()
{
    // Declare the vertical and horizontal resolution variables.
    int hRes = *(int*)((intptr_t)baseModule + 0x316A338); // Grabs Horizontal Resolution integer.
    int vRes = *(int*)((intptr_t)baseModule + 0x316A33C); // Grabs Vertical Resolution integer.

    // Convert the int values to floats, so then we can determine the aspect ratio.
    float aspectRatio = (float)hRes / (float)vRes;

    switch (useCustomFOV)
    {
        case 0:
        {
            // If useCustomFOV is set to "0", then calculate the vertical FOV using the new aspect ratio, the old aspect ratio, and the original FOV.
            FOV = round((2.0f * atan(((aspectRatio) / (16.0f / 9.0f)) * tan((originalFOV * 10000.0f) / 2.0f * ((float)M_PI / 180.0f)))) * (180.0f / (float)M_PI) * 100.0f) / 100.0f / 10000.0f;
            break;
        }
        case 1:
        {
			// Subtracts the custom FOV by the default FOV to get the difference
			float FOVDifference = (float)customFOV - 90.0f;
            // If useCustomFOV is set to "1", then calculate the vertical FOV using the new aspect ratio, the old aspect ratio, and the desired custom FOV (based on the FOVDifference to offset any oddities).
            FOV = round((2.0f * atan(((aspectRatio) / (16.0f / 9.0f)) * tan(((originalFOV * 10000.0f) + FOVDifference) / 2.0f * ((float)M_PI / 180.0f)))) * (180.0f / (float)M_PI) * 100.0f) / 100.0f / 10000.0f;
            break;
        }
    }

    // Writes FOV to Memory.
    *(float*)((intptr_t)baseModule + 0x264ED20) = FOV;
}

void uncapFPS() //Uncaps the framerate.
{
	//Writes the new t.MaxFPS cap to memory, alongside pointer.
	*(float*)(*((intptr_t*)((intptr_t)baseModule + 0x03416F50)) + 0x0) = (float)tMaxFPS;
}

void resolutionCheck()
{
    if (aspectRatio != (*(int*)((intptr_t)baseModule + 0x316A338) / *(int*)((intptr_t)baseModule + 0x316A33C)))
    {
        fovCalc();
    }
}

void framerateCheck()
{
	if (tMaxFPS != *(float*)(*((intptr_t*)((intptr_t)baseModule + 0x03416F50)) + 0x0))
	{
		uncapFPS();
	}
}

void StartPatch()
{
    // Reads the "config.ini" config file for values that we are going to want to modify.
    readConfig();

	// Unprotects the main module handle.
	ScopedUnprotect::FullModule UnProtect(baseModule);;

    Sleep(5000); // Sleeps the thread for five seconds before applying the memory values.

	fovCalc(); // Calculates the new vertical FOV.

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