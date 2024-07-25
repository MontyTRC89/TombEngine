#pragma once

#include "Math/Math.h"
#include "Objects/objectslist.h"

using namespace TEN::Math;

struct StaticAsset;

class StaticObject
{
public:
	std::string	   Name	   = {};
	int			   ID	   = 0;
	GAME_OBJECT_ID AssetID = GAME_OBJECT_ID::ID_NO_OBJECT;

	Pose  Pose		 = Pose::Zero;
	int	  RoomNumber = 0;
	float Scale		 = 0.0f;
	Color Color		 = {};

	int	 HitPoints = 0;
	int	 Flags	   = 0;
	bool IsDirty   = false;

	const StaticAsset& GetAsset() const;
};
