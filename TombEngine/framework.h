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

#include <algorithm>
#include <ctime>
#include <array>
#include <deque>
#include <execution>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <locale>
#include <map>
#include <memory>
#include <optional>
#include <random>
#include <regex>
#include <set>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <process.h>
#include <wincodec.h>

// TODO: Remove this and everything that relies on it.
#include <codecvt>

// Lua
#include <sol/sol.hpp>

// BASS
#include <bass.h>
#include <bass_fx.h>

// DirectX
#include <directxtk/CommonStates.h>
#include <d3d11.h>
#include <d3d9types.h>
#include <d3dcompiler.h>
#include <directxtk/DDSTextureLoader.h>
#include <directxtk/PrimitiveBatch.h>
#include <directxtk/PostProcess.h>
#include <directxtk/SpriteFont.h>
#include <directxtk/ScreenGrab.h>
#include <directxtk/WICTextureLoader.h>

// DirectX Tool Kit
#include <directxtk/SimpleMath.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;

// OIS
#include <ois/OIS.h>

// WRL
#include <wrl/client.h>

// zlib
#include <zlib.h>

// Resources
#include "Math/Math.h"
#include "Resource.h"

using namespace TEN::Math;

// Debug 
#include "Game/Debug/Debug.h"

using namespace TEN::Debug;

// Constants

constexpr auto NO_VALUE = -1;
