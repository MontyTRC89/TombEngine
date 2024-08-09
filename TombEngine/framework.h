#pragma once
// NOTE: Disable Unicode for FMT and SPDLOG, else cause error when compiling...
#define FMT_UNICODE 0

// Windows
#pragma comment(lib, "comctl32")
#pragma comment(lib, "version")

// Spdlog, Zlib, OIS library
#if defined(_DEBUG)
#pragma comment(lib, "spdlogd")
#pragma comment(lib, "zlibd")
#pragma comment(lib, "OIS_d")
#else
#pragma comment(lib, "spdlog")
#pragma comment(lib, "zlib")
#pragma comment(lib, "OIS")
#endif

// Lua library
#pragma comment(lib, "lua-c++")

// DirectXTK library + DirectX11
#pragma comment(lib, "d3dcompiler")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "DirectXTK")

// Bass library
#pragma comment(lib, "bass")
#pragma comment(lib, "bass_fx")
#pragma comment(lib, "bassmix")

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <istream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <array>
#include <deque>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <filesystem>
#include <wincodec.h>
#include <d3d9types.h>
#include <d3d11.h>
#include <wrl/client.h>

#include <ois/OISKeyboard.h>
#include <ois/OISException.h>
#include <ois/OISForceFeedback.h>
#include <ois/OISInputManager.h>
#include <ois/OISJoyStick.h>
#include <ois/OISKeyboard.h>
#include <ois/OISMouse.h>
#include <sol/sol.hpp>
#include <DirectXCollision.h>
#include <directxtk/CommonStates.h>
#include <directxtk/SpriteFont.h>
#include <directxtk/PrimitiveBatch.h>
#include <directxtk/PostProcess.h>
#include <directxtk/DDSTextureLoader.h>
#include <directxtk/SimpleMath.h>
#include <directxtk/ScreenGrab.h>
#include <directxtk/WICTextureLoader.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;

#include "Game/Debug/Debug.h"

using namespace TEN::Debug;

constexpr auto NO_VALUE = -1;

