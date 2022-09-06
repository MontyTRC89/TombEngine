#include "framework.h"
#include "Math/Containers/PoseData.h"

#include "Math/Containers/Vector3i.h"
#include "Math/Containers/Vector3s.h"

//using namespace TEN::Math::Angles;

//namespace TEN::Math
//{
	PHD_3DPOS const PHD_3DPOS::Empty = PHD_3DPOS(Vector3i::Zero, Vector3Shrt::Zero);

	PHD_3DPOS::PHD_3DPOS()
	{
	}

	PHD_3DPOS::PHD_3DPOS(const Vector3i& pos)
	{
		this->Position = pos;
	}

	PHD_3DPOS::PHD_3DPOS(int xPos, int yPos, int zPos)
	{
		this->Position = Vector3i(xPos, yPos, zPos);
	}

	PHD_3DPOS::PHD_3DPOS(const Vector3Shrt& orient)
	{
		this->Orientation = orient;
	}

	PHD_3DPOS::PHD_3DPOS(short xOrient, short yOrient, short zOrient)
	{
		this->Orientation = Vector3Shrt(xOrient, yOrient, zOrient);
	}

	PHD_3DPOS::PHD_3DPOS(const Vector3i& pos, const Vector3Shrt& orient)
	{
		this->Position = pos;
		this->Orientation = orient;
	}

	PHD_3DPOS::PHD_3DPOS(const Vector3i& pos, short xOrient, short yOrient, short zOrient)
	{
		this->Position = pos;
		this->Orientation = Vector3Shrt(xOrient, yOrient, zOrient);
	}

	PHD_3DPOS::PHD_3DPOS(int xPos, int yPos, int zPos, const Vector3Shrt& orient)
	{
		this->Position = Vector3i(xPos, yPos, zPos);
		this->Orientation = orient;
	}

	PHD_3DPOS::PHD_3DPOS(int xPos, int yPos, int zPos, short xOrient, short yOrient, short zOrient)
	{
		this->Position = Vector3i(xPos, yPos, zPos);
		this->Orientation = Vector3Shrt(xOrient, yOrient, zOrient);
	}
//}
