#pragma once
#include <deque>

#include "Game/room.h"

struct CollisionResult;

namespace TEN::Effects::Blood
{
	constexpr auto BLOOD_DRIP_NUM_MAX  = 512;
	constexpr auto BLOOD_STAIN_NUM_MAX = 192;

	constexpr auto BLOOD_DRIP_SPRAY_NUM_DEFAULT = 2;

	struct BloodDrip
	{
		bool		 IsActive	   = false;
		unsigned int SpriteIndex   = 0;

		Vector3 Position   = Vector3::Zero;
		int		RoomNumber = NO_ROOM;
		Vector3 Velocity   = Vector3::Zero;
		Vector4 Color	   = Vector4::Zero;

		float Life	  = 0.0f;
		float Scale	  = 0.0f;
		float Gravity = 0.0f;
	};

	struct BloodStain
	{
		unsigned int SpriteIndex = 0;

		Vector3	Position	  = Vector3::Zero;
		int		RoomNumber	  = NO_ROOM;
		short	Orientation2D = 0;
		Vector3 Normal		  = Vector3::Zero;
		Vector4	Color		  = Vector4::Zero;
		Vector4	ColorStart	  = Vector4::Zero;
		Vector4	ColorEnd	  = Vector4::Zero;

		std::array<Vector3, 4> VertexPoints = {};

		float Life			  = 0.0f;
		float LifeStartFading = 0.0f;

		float Scale		= 0.0f;
		float ScaleMax	= 0.0f;
		float ScaleRate = 0.0f;

		float Opacity	   = 0.0f;
		float OpacityStart = 0.0f;

		float DelayTime = 0.0f;
	};

	extern std::array<BloodDrip, BLOOD_DRIP_NUM_MAX> BloodDrips;
	extern std::deque<BloodStain>					 BloodStains;

	BloodDrip&			   GetFreeBloodDrip();
	std::array<Vector3, 4> GetBloodStainVertexPoints(const Vector3& pos, short orient2D, const Vector3& normal, float scale);

	bool TestBloodStainFloor(const BloodStain& stain);

	void SpawnBloodMist(const Vector3& pos, int roomNumber, const Vector3& direction, unsigned int count);
	void SpawnBloodMistCloud(const Vector3& pos, int roomNumber, const Vector3& direction, float velocity, unsigned int count);
	void SpawnBloodMistCloudUnderwater(const Vector3& pos, int roomNumber, float velocity);

	void SpawnBloodDrip(const Vector3& pos, int roomNumber, const Vector3& velocity, float scale);
	void SpawnBloodDripSpray(const Vector3& pos, int roomNumber, const Vector3& direction, const Vector3& baseVelocity, unsigned int count = BLOOD_DRIP_SPRAY_NUM_DEFAULT);

	void SpawnBloodStain(const Vector3& pos, int roomNumber, const Vector3& normal, float scaleMax, float scaleRate, float delayTimeInSec = 0.0f);
	void SpawnBloodStainFromDrip(const BloodDrip& drip, const CollisionResult& pointColl);
	void SpawnBloodStainPool(ItemInfo& item);

	void UpdateBloodMists();
	void UpdateBloodDrips();
	void UpdateBloodStains();
	void ClearBloodMists();
	void ClearBloodDrips();
	void ClearBloodStains();

	void DrawBloodDebug();
}
