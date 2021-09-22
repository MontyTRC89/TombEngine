#pragma once
#include "phd_global.h"
#include <vector>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

struct MUTATOR
{
	int MeshIndex = -1;
	Vector3 Scale    = Vector3::One;
	Vector3 Offset   = Vector3::Zero;
	Vector4 Color    = Vector3::One;
	Vector3 Rotation = Vector3::Zero;
};

struct MUTATOR_INFO
{
	std::vector<MUTATOR> Nodes;
};