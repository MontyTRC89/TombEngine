#include "framework.h"
#include "Math/PoseData.h"

#include "Math/Angles/EulerAngles.h"
#include "Math/Vector3i.h"

//using namespace TEN::Math::Angles;

//namespace TEN::Math
//{
	PHD_3DPOS::PHD_3DPOS()
	{
		this->Position = Vector3Int::Zero;
		this->Orientation = EulerAngles::Zero;
	}

	PHD_3DPOS::PHD_3DPOS(Vector3Int pos)
	{
		this->Position = pos;
		this->Orientation = EulerAngles::Zero;
	}

	PHD_3DPOS::PHD_3DPOS(int xPos, int yPos, int zPos)
	{
		this->Position = Vector3Int(xPos, yPos, zPos);
		this->Orientation = EulerAngles::Zero;
	}

	PHD_3DPOS::PHD_3DPOS(EulerAngles orient)
	{
		this->Position = Vector3Int::Zero;
		this->Orientation = orient;
	}

	PHD_3DPOS::PHD_3DPOS(float xOrient, float yOrient, float zOrient)
	{
		this->Position = Vector3Int::Zero;
		this->Orientation = EulerAngles(xOrient, yOrient, zOrient);
	}

	PHD_3DPOS::PHD_3DPOS(Vector3Int pos, EulerAngles orient)
	{
		this->Position = pos;
		this->Orientation = orient;
	}

	PHD_3DPOS::PHD_3DPOS(Vector3Int pos, float xOrient, float yOrient, float zOrient)
	{
		this->Position = pos;
		this->Orientation = EulerAngles(xOrient, yOrient, zOrient);
	}

	PHD_3DPOS::PHD_3DPOS(int xPos, int yPos, int zPos, EulerAngles orient)
	{
		this->Position = Vector3Int(xPos, yPos, zPos);
		this->Orientation = orient;
	}

	PHD_3DPOS::PHD_3DPOS(int xPos, int yPos, int zPos, float xOrient, float yOrient, float zOrient)
	{
		this->Position = Vector3Int(xPos, yPos, zPos);
		this->Orientation = EulerAngles(xOrient, yOrient, zOrient);
	}
//}
