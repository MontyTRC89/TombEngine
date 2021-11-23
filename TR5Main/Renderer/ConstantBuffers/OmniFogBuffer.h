#pragma once
#include <SimpleMath.h>
using namespace DirectX::SimpleMath;

struct alignas(16) COmniFogBuffer
{
	Vector4 FogColor;
	int FogMinDistance;
	int FogMaxDistance;
	float Padding[2];
};
