#pragma once

class Frustum
{
public:

	Frustum() = default;

	void Update(const Matrix& view, const Matrix& projection);
	bool PointInFrustum(const Vector3& position) const;
	bool SphereInFrustum(const Vector3& position, float radius) const;
	bool AABBInFrustum(const Vector3& min, const Vector3& max) const;

private:

	void NormalizePlane(int32_t side);
	std::array<std::array<float, 4>, 6> m_frustum = {};

};
