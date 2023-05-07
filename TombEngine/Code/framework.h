#pragma once

//#define _XM_NO_INTRINSICS_ 1

#include <algorithm>
#include <array>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sol.hpp>
#include <stdint.h>
#include <stdio.h>
#include <filesystem>
#include <stdlib.h>
#include <string>
#include <vector>
#include <unordered_set>
#include <variant>
#include <thread>
#include <stdexcept>
#include <string_view>
#include <random>
#include <regex>
#include <iostream>
#include <fstream>
#include <future>
#include <execution>
#include <stack>
#include <tuple>
#include <locale>

#include <codecvt>	// TODO: remove this and everything that relies on it

// DX includes

#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3d9types.h>
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include <CommonStates.h>
#include <SpriteFont.h>
#include <PrimitiveBatch.h>

// WRL includes

#include <wrl/client.h>

// BASS includes

#include <bass.h>
#include <bass_fx.h>

// OIS includes

#include <OISKeyboard.h>

// zlib includes

#include <zlib.h>

// debug include

#include <Game/debug/debug.h>

// DXTK includes

#include <SimpleMath.h>

//#include <Math/Math.h> // TODO

using namespace DirectX;
using namespace DirectX::SimpleMath;

#if __cplusplus >= 202002L
#define USE_FEATURE_IF_CPP20(x) x
#else
#define USE_FEATURE_IF_CPP20(x)
#endif