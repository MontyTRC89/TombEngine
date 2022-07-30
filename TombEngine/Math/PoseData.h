#pragma once
#include "Math/Angles/EulerAngles.h"
#include "Math/Vector3i.h"

struct PHD_3DPOS
{
	Vector3Int  Position	= Vector3Int::Zero;
	EulerAngles Orientation = EulerAngles::Zero;

	PHD_3DPOS();
	PHD_3DPOS(Vector3Int pos);
	PHD_3DPOS(int xPos, int yPos, int zPos);
	PHD_3DPOS(EulerAngles orient);
	PHD_3DPOS(float xOrient, float yOrient, float zOrient);
	PHD_3DPOS(Vector3Int pos, EulerAngles orient);
	PHD_3DPOS(Vector3Int pos, float xOrient, float yOrient, float zOrient);
	PHD_3DPOS(int xPos, int yPos, int zPos, EulerAngles orient);
	PHD_3DPOS(int xPos, int yPos, int zPos, float xOrient, float yOrient, float zOrient);
};
