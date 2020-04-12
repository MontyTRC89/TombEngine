#pragma once
#include <d3d11.h>
#include <SimpleMath.h>
using namespace DirectX::SimpleMath;
DirectX::SimpleMath::Matrix inverseOrtho(const DirectX::SimpleMath::Matrix& m);
struct Frustum {

	Vector3 pos;
	Vector4 planes[32];   // + buffer for OBB visibility test
	int  start, count;

	void calcPlanes(const Matrix& m);
	// AABB visibility check
	bool isVisible(const Vector3& min, const Vector3& max) const;
	// OBB visibility check
	bool isVisible(const Matrix& matrix, const Vector3& min, const Vector3& max);
	// Sphere visibility check
	bool isVisible(const Vector3& center, float radius);
};