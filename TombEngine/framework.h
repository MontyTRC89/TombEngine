#pragma once

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
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

// TODO: Remove this and everything that relies on it.
#include <codecvt>

// BASS
#include <bass.h>
#include <bass_fx.h>

// DirectX
#include <CommonStates.h>
#include <d3d11.h>
#include <d3d9types.h>
#include <d3dcompiler.h>
#include <DDSTextureLoader.h>
#include <PrimitiveBatch.h>
#include <SpriteFont.h>
#include <WICTextureLoader.h>

// DirectX Tool Kit
#include <SimpleMath.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;

// OIS
#include <OISKeyboard.h>

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
