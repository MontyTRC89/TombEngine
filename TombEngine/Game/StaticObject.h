#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

struct StaticAsset;

class StaticObject
{
public:
	std::string		  Name	   = {};
	int				  ID	   = 0;
	const StaticAsset* AssetPtr = nullptr;

	Pose  Pose		 = Pose::Zero;
	int	  RoomNumber = 0;
	float Scale		 = 0.0f;
	Color Color		 = {};

	int	 HitPoints = 0;
	int	 Flags	   = 0;
	bool IsDirty   = false;
};
