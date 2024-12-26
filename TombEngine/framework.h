#pragma once

// Disable Unicode for FMT and spdlog to prevent errors when compiling.
#define FMT_UNICODE 0

// =========
// LIBRARIES
// =========

// Windows
#pragma comment(lib, "comctl32")
#pragma comment(lib, "version")

// BASS
#pragma comment(lib, "bass")
#pragma comment(lib, "bass_fx")
#pragma comment(lib, "bassmix")

// DirectX 11
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "dxguid")

// DirectX Tool Kit
#pragma comment(lib, "DirectXTK")

// Lua
#pragma comment(lib, "lua-c++")

// OIS, spdlog, zlib
#if defined(_DEBUG)
#pragma comment(lib, "OIS_d")
#pragma comment(lib, "spdlogd")
#pragma comment(lib, "zlibd")
#else
#pragma comment(lib, "OIS")
#pragma comment(lib, "spdlog")
#pragma comment(lib, "zlib")
#endif

// ========
// INCLUDES
// ========

// Standard
#include <algorithm>
#include <array>
#include <chrono>
#include <CommCtrl.h>
#include <ctime>
#include <deque>
#include <execution>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <locale>
#include <map>
#include <memory>
#include <optional>
#include <process.h>
#include <random>
#include <regex>
#include <set>
#include <srtparser.h>
#include <stack>
#include <stdarg.h>
#include <stdexcept>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>
#include <wincodec.h>

// BASS
#include <bass.h>
#include <bass_fx.h>

// DirectX 11
#include <d3d11.h>
#include <d3d9types.h>
#include <d3dcompiler.h>
#include <DirectXCollision.h>

// DirectX Tool Kit
#include <directxtk/CommonStates.h>
#include <directxtk/DDSTextureLoader.h>
#include <directxtk/PostProcess.h>
#include <directxtk/PrimitiveBatch.h>
#include <directxtk/ScreenGrab.h>
#include <directxtk/SimpleMath.h>
#include <directxtk/SpriteFont.h>
#include <directxtk/WICTextureLoader.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;

// Lua
#include <sol/sol.hpp>

// OIS
#include <ois/OIS.h>

// spdlog
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

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

// =========
// CONSTANTS
// =========

constexpr auto NO_VALUE = -1;
