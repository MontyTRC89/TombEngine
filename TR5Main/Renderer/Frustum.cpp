#include "Frustum.h"



DirectX::SimpleMath::Matrix inverseOrtho(const DirectX::SimpleMath::Matrix& m)
{
	DirectX::SimpleMath::Matrix r;
	r._11 = m._11; r._21 = m._12; r._31 = m._13; r._41 = 0;
	r._12 = m._21; r._22 = m._22; r._32 = m._23; r._42 = 0;
	r._13 = m._31; r._23 = m._32; r._33 = m._33; r._43 = 0;
	r._14 = -(m._14 * m._11 + m._24 * m._21 + m._34 * m._31); // -dot(pos, right)
	r._24 = -(m._14 * m._12 + m._24 * m._22 + m._34 * m._32); // -dot(pos, up)
	r._34 = -(m._14 * m._13 + m._24 * m._23 + m._34 * m._33); // -dot(pos, dir)
	r._44 = 1;

	return r;
}

void Frustum::calcPlanes(const Matrix& mm)
{
	Matrix m = mm.Transpose();
	start = 0;

	count = 5;

	planes[0] = Vector4(m._41 - m._31, m._42 - m._32, m._43 - m._33, m._44 - m._34); // near
	planes[1] = Vector4(m._41 - m._21, m._42 - m._22, m._43 - m._23, m._44 - m._24); // top
	planes[2] = Vector4(m._41 - m._11, m._42 - m._12, m._43 - m._13, m._44 - m._14); // right
	planes[3] = Vector4(m._41 + m._21, m._42 + m._22, m._43 + m._23, m._44 + m._24); // bottom
	planes[4] = Vector4(m._41 + m._11, m._42 + m._12, m._43 + m._13, m._44 + m._14); // left

	for (int i = 0; i < count; i++)

		planes[i] *= 1.0f / DirectX::SimpleMath::Vector3(planes[i]).Length();
}

bool Frustum::isVisible(const Vector3& min, const Vector3& max) const
{
	if (count < 4) return false;
	for (int i = start; i < start + count; i++) {
		const Vector3& n = Vector3(planes[i]);
		const float d = -planes[i].w;
		if (n.Dot(max) < d &&
			n.Dot(min) < d &&
			n.Dot(Vector3(min.x, max.y, max.z)) < d &&
			n.Dot(Vector3(max.x, min.y, max.z)) < d &&
			n.Dot(Vector3(min.x, min.y, max.z)) < d &&
			n.Dot(Vector3(max.x, max.y, min.z)) < d &&
			n.Dot(Vector3(min.x, max.y, min.z)) < d &&
			n.Dot(Vector3(max.x, min.y, min.z)) < d)
			return false;
	}
	return true;
}

bool Frustum::isVisible(const Matrix& matrix, const Vector3& min, const Vector3& max)
{
	start = count;

	// transform clip planes (relative)

	Matrix m = inverseOrtho(matrix);
	for (int i = 0; i < count; i++) {
		Vector4& p = planes[i];
		Vector4 o = Vector4::Transform(Vector4(p.x * (-p.w), p.y * (-p.w), p.z * (-p.w), 1.0f), m);
		Vector4 n = Vector4::Transform(Vector4(p.x, p.y, p.z, 0), m);
		planes[start + i] = Vector4(n.x, n.y, n.z, -Vector3(n).Dot(Vector3(o)));

	}

	bool visible = isVisible(min, max);

	start = 0;

	return visible;
}

bool Frustum::isVisible(const Vector3& center, float radius)
{
	if (count < 4) return false;

	for (int i = 0; i < count; i++)
		if (Vector3(planes[i]).Dot(center) + planes[i].w < -radius)
			return false;
	return true;
}
