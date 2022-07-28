#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <memory>
#include <functional>
#include <vector>
#include <map>
#include <string>
#include <array>
#include <d3d11.h>
#include <optional>
#include <SimpleMath.h>
#include "Game/debug/debug.h"
#include <algorithm>
#include <set>
#include <sol.hpp>

#if __cplusplus >= 202002L
#define USE_FEATURE_IF_CPP20(x) x
#else
#define USE_FEATURE_IF_CPP20(x)
#endif

using namespace DirectX;
using namespace DirectX::SimpleMath;

#pragma warning(disable: 4996)
