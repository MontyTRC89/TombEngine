#pragma once

//#define _XM_NO_INTRINSICS_ 1

#include <algorithm>
#include <array>
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
#include <random>
#include <regex>
#include <set>
#include <sol.hpp>
#include <stack>
#include <stdexcept>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <unordered_set>
#include <variant>
#include <vector>

#include <codecvt>	// TODO: Remove this and everything that relies on it.

// DX includes
#include <CommonStates.h>
#include <d3d11.h>
#include <d3d9types.h>
#include <d3dcompiler.h>
#include <DDSTextureLoader.h>
#include <PrimitiveBatch.h>
#include <SpriteFont.h>
#include <WICTextureLoader.h>

// WRL includes
#include <wrl/client.h>

// BASS includes
#include <bass.h>
#include <bass_fx.h>

// OIS includes
#include <OISKeyboard.h>

// zlib includes
#include <zlib.h>

// DXTK includes
#include <SimpleMath.h>

// Resource includes
//#include "Math/Math.h" // TODO
#include "resource.h"

// Debug includes
#include "Game/debug/debug.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

#if __cplusplus >= 202002L
#define USE_FEATURE_IF_CPP20(x) x
#else
#define USE_FEATURE_IF_CPP20(x)
#endif
