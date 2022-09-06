#pragma once
#include "Math/Angles/EulerAngles.h"
#include "Math/Containers/Vector3i.h"

//namespace TEN::Math
//{
	// TODO: Rename to PoseData (or something else that specifically describes a position + orientation representation, if we prefer).
	// This struct has changed vastly and the old name is no longer appropriate. -- Sezz 2022.07.23
	struct PHD_3DPOS
	{
		Vector3i	Position	= Vector3i::Zero;
		EulerAngles Orientation = EulerAngles::Zero;

		static const PHD_3DPOS Empty;

		PHD_3DPOS();
		PHD_3DPOS(const Vector3i& pos);
		PHD_3DPOS(int xPos, int yPos, int zPos);
		PHD_3DPOS(const EulerAngles& orient);
		PHD_3DPOS(short xOrient, short yOrient, short zOrient);
		PHD_3DPOS(const Vector3i& pos, const EulerAngles& orient);
		PHD_3DPOS(const Vector3i& pos, short xOrient, short yOrient, short zOrient);
		PHD_3DPOS(int xPos, int yPos, int zPos, const EulerAngles& orient);
		PHD_3DPOS(int xPos, int yPos, int zPos, short xOrient, short yOrient, short zOrient);
	};
//}
