#pragma once
#include "Game/debug/debug.h"

#include <algorithm>
#include <array>
#include <d3d11.h>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <SimpleMath.h>
#include <set>
#include <sol.hpp>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#if __cplusplus >= 202002L
#define USE_FEATURE_IF_CPP20(x) x
#else
#define USE_FEATURE_IF_CPP20(x)
#endif

using namespace DirectX;
using namespace DirectX::SimpleMath;

#pragma warning(disable: 4996)
