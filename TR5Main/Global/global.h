#pragma once

#include "vodoo.h"
#include "constants.h"
#include "enums.h"
#include "objectslist.h"
#include "types.h"
#include "vars.h"
#include "math.h"
#include "macros.h"
#include "malloc.h"
#include <windows.h>"
#include "..\Scripting\GameLogicScript.h"
#include "..\Scripting\GameFlowScript.h"
#include "..\Renderer\Renderer.h"

#define DB_Log ((HWND (__cdecl*)(__int16, const char*)) 0x004DEB10)

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)

extern HMODULE g_DllHandle;