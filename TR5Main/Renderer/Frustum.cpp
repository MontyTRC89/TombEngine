#include "framework.h"
#include "Frustum.h"

void Frustum::Update(const Matrix& view, const Matrix& projection)
{
	std::array<float, 16> clip;
	clip[0] = view._11 * projection._11 + view._12 * projection._21 + view._13 * projection._31 + view._14 * projection._41;
	clip[1] = view._11 * projection._12 + view._12 * projection._22 + view._13 * projection._32 + view._14 * projection._42;
	clip[2] = view._11 * projection._13 + view._12 * projection._23 + view._13 * projection._33 + view._14 * projection._43;
	clip[3] = view._11 * projection._14 + view._12 * projection._24 + view._13 * projection._34 + view._14 * projection._44;

	clip[4] = view._21 * projection._11 + view._22 * projection._21 + view._23 * projection._31 + view._24 * projection._41;
	clip[5] = view._21 * projection._12 + view._22 * projection._22 + view._23 * projection._32 + view._24 * projection._42;
	clip[6] = view._21 * projection._13 + view._22 * projection._23 + view._23 * projection._33 + view._24 * projection._43;
	clip[7] = view._21 * projection._14 + view._22 * projection._24 + view._23 * projection._34 + view._24 * projection._44;

	clip[8] = view._31 * projection._11 + view._32 * projection._21 + view._33 * projection._31 + view._34 * projection._41;
	clip[9] = view._31 * projection._12 + view._32 * projection._22 + view._33 * projection._32 + view._34 * projection._42;
	clip[10] = view._31 * projection._13 + view._32 * projection._23 + view._33 * projection._33 + view._34 * projection._43;
	clip[11] = view._31 * projection._14 + view._32 * projection._24 + view._33 * projection._34 + view._34 * projection._44;
		
	clip[12] = view._41 * projection._11 + view._42 * projection._21 + view._43 * projection._31 + view._44 * projection._41;
	clip[13] = view._41 * projection._12 + view._42 * projection._22 + view._43 * projection._32 + view._44 * projection._42;
	clip[14] = view._41 * projection._13 + view._42 * projection._23 + view._43 * projection._33 + view._44 * projection._43;
	clip[15] = view._41 * projection._14 + view._42 * projection._24 + view._43 * projection._34 + view._44 * projection._44;
	
	// This will extract the LEFT side of the frustum.
	m_frustum[1][0] = clip[3] - clip[0];
	m_frustum[1][1] = clip[7] - clip[4];
	m_frustum[1][2] = clip[11] - clip[8];
	m_frustum[1][3] = clip[15] - clip[12];
	NormalizePlane(1);

	// This will extract the RIGHT side of the frustum.
	m_frustum[0][0] = clip[3] + clip[0];
	m_frustum[0][1] = clip[7] + clip[4];
	m_frustum[0][2] = clip[11] + clip[8];
	m_frustum[0][3] = clip[15] + clip[12];
	NormalizePlane(0);

	// This will extract the BOTTOM side of the frustum.
	m_frustum[2][0] = clip[3] + clip[1];
	m_frustum[2][1] = clip[7] + clip[5];
	m_frustum[2][2] = clip[11] + clip[9];
	m_frustum[2][3] = clip[15] + clip[13];
	NormalizePlane(2);
	
	// This will extract the TOP side of the frustum.
	m_frustum[3][0] = clip[3] - clip[1];
	m_frustum[3][1] = clip[7] - clip[5];
	m_frustum[3][2] = clip[11] - clip[9];
	m_frustum[3][3] = clip[15] - clip[13];
	NormalizePlane(3);

	// This will extract the BACK side of the frustum.
	m_frustum[4][0] = clip[3] + clip[2];
	m_frustum[4][1] = clip[7] + clip[6];
	m_frustum[4][2] = clip[11] + clip[10];
	m_frustum[4][3] = clip[15] + clip[14];
	NormalizePlane(4);

	// This will extract the FRONT side of the frustum.
	m_frustum[5][0] = clip[3] - clip[2];
	m_frustum[5][1] = clip[7] - clip[6];
	m_frustum[5][2] = clip[11] - clip[10];
	m_frustum[5][3] = clip[15] - clip[14];
	NormalizePlane(5);
}

bool Frustum::PointInFrustum(const Vector3& position) const
{
	for (uint32_t i = 0; i < 6; i++)
	{
		if (m_frustum[i][0] * position.x + m_frustum[i][1] * position.y + m_frustum[i][2] * position.z + m_frustum[i][3] <= 0.0f)
			return false;
	}

	return true;
}

bool Frustum::SphereInFrustum(const Vector3& position, float radius) const
{
	for (uint32_t i = 0; i < 6; i++)
	{
		if (m_frustum[i][0] * position.x + m_frustum[i][1] * position.y + m_frustum[i][2] * position.z + m_frustum[i][3] <= -radius)
			return false;
	}

	return true;
}

bool Frustum::AABBInFrustum(const Vector3& min, const Vector3& max) const
{
	for (uint32_t i = 0; i < 6; i++)
	{
		if (m_frustum[i][0] * min.x + m_frustum[i][1] * min.y + m_frustum[i][2] * min.z + m_frustum[i][3] <= 0.0f &&
			m_frustum[i][0] * max.x + m_frustum[i][1] * min.y + m_frustum[i][2] * min.z + m_frustum[i][3] <= 0.0f &&
			m_frustum[i][0] * min.x + m_frustum[i][1] * max.y + m_frustum[i][2] * min.z + m_frustum[i][3] <= 0.0f &&
			m_frustum[i][0] * max.x + m_frustum[i][1] * max.y + m_frustum[i][2] * min.z + m_frustum[i][3] <= 0.0f &&
			m_frustum[i][0] * min.x + m_frustum[i][1] * min.y + m_frustum[i][2] * max.z + m_frustum[i][3] <= 0.0f &&
			m_frustum[i][0] * max.x + m_frustum[i][1] * min.y + m_frustum[i][2] * max.z + m_frustum[i][3] <= 0.0f &&
			m_frustum[i][0] * min.x + m_frustum[i][1] * max.y + m_frustum[i][2] * max.z + m_frustum[i][3] <= 0.0f &&
			m_frustum[i][0] * max.x + m_frustum[i][1] * max.y + m_frustum[i][2] * max.z + m_frustum[i][3] <= 0.0f)
		{
			return false;
		}
	}
	return true;
}

void Frustum::NormalizePlane(int32_t side)
{
	float magnitude = sqrt(m_frustum[side][0] * m_frustum[side][0] + m_frustum[side][1] * m_frustum[side][1] + m_frustum[side][2] * m_frustum[side][2]);
	m_frustum[side][0] /= magnitude;
	m_frustum[side][1] /= magnitude;
	m_frustum[side][2] /= magnitude;
	m_frustum[side][3] /= magnitude;
}
