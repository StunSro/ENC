#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _CRT_SECURE_NO_WARNINGS

// Windows Header Files
#include <windows.h>
#include <iostream>
#include <list>

#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "VMProtectSDK.h"
#pragma comment(lib, "VMProtectSDK32.lib")

#include "detours.h"
#pragma comment(lib, "detours.lib")