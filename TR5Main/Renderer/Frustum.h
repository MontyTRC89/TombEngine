#pragma once
#include <d3d11.h>
#include <SimpleMath.h>
#include <array>

class Frustum {

public:

	Frustum() = default;

	void Update(const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& projection);
	bool PointInFrustum(const DirectX::SimpleMath::Vector3& position) const;
	bool SphereInFrustum(const DirectX::SimpleMath::Vector3& position, float radius) const;
	bool AABBInFrustum(const DirectX::SimpleMath::Vector3& min, const DirectX::SimpleMath::Vector3& max) const;

private:

	void NormalizePlane(int32_t side);
	std::array<std::array<float, 4>, 6> m_frustum = {};

};