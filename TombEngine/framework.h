#pragma once
#include <algorithm>
#include <array>
#include <d3d11.h>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <SimpleMath.h>
#include <sol.hpp>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "Game/debug/debug.h"

#if __cplusplus >= 202002L
#define USE_FEATURE_IF_CPP20(x) x
#else
#define USE_FEATURE_IF_CPP20(x)
#endif

#pragma warning(disable: 4996)

using namespace DirectX;
using namespace DirectX::SimpleMath;

typedef unsigned char	   uchar;
typedef unsigned short	   ushort;
typedef unsigned int	   uint;
typedef unsigned long	   ulong;
typedef unsigned long long ulonglong;
