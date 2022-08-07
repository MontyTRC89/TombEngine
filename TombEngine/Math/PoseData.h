#pragma once
#include "Math/Angles/Vector3s.h"
#include "Math/Vector3i.h"

//using namespace TEN::Math;
//using namespace TEN::Math::Angles;

//namespace TEN::Math
//{
	// TODO: Rename to PoseData (or something else that specifically describes a position + orientation representation, if we prefer).
	// This struct has changed vastly and the old name is no longer appropriate. -- Sezz 2022.07.23
	struct PHD_3DPOS
	{
		Vector3Int  Position	= Vector3Int::Zero;
		Vector3Shrt Orientation = Vector3Shrt::Zero;

		PHD_3DPOS();
		PHD_3DPOS(Vector3Int pos);
		PHD_3DPOS(int xPos, int yPos, int zPos);
		PHD_3DPOS(Vector3Shrt orient);
		PHD_3DPOS(short xOrient, short yOrient, short zOrient);
		PHD_3DPOS(Vector3Int pos, Vector3Shrt orient);
		PHD_3DPOS(Vector3Int pos, short xOrient, short yOrient, short zOrient);
		PHD_3DPOS(int xPos, int yPos, int zPos, Vector3Shrt orient);
		PHD_3DPOS(int xPos, int yPos, int zPos, short xOrient, short yOrient, short zOrient);
	};
//}
