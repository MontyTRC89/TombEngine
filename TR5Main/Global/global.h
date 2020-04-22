#pragma once

#include <windows.h>
#include <D3D11.h>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

#include "vodoo.h"
#include "constants.h"
#include "enums.h"
#include "objectslist.h"
#include "types.h"
#include "vars.h"
#include "math.h"
#include "macros.h"
#include "malloc.h"
#include "legacyfunctions.h"

#include "..\Scripting\GameLogicScript.h"
#include "..\Scripting\GameFlowScript.h"

#include "..\Renderer\Renderer11.h"

//#define DB_Log ((HWND (__cdecl*)(short, const char*)) 0x004DEB10)

#pragma warning(disable:4996)

extern HMODULE g_DllHandle;