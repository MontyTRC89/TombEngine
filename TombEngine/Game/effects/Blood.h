#pragma once
#include <deque>

#include "Game/room.h"

namespace TEN::Effects::Blood
{
	constexpr auto BLOOD_DRIP_NUM_MAX  = 128;
	constexpr auto BLOOD_STAIN_NUM_MAX = 128;

	constexpr auto BLOOD_DRIP_SPRAY_NUM_DEFAULT = 3;

	struct BloodDrip
	{
		bool IsActive = false;

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
		Vector3	Position   = Vector3::Zero;
		int		RoomNumber = NO_ROOM;
		Vector3 Normal	   = Vector3::Zero;
		Vector4	Color	   = Vector4::Zero;
		Vector4	ColorStart = Vector4::Zero;
		Vector4	ColorEnd   = Vector4::Zero;

		float Life			  = 0.0f;
		float LifeStartFading = 0.0f;

		float Scale		= 0.0f;
		float ScaleMax	= 0.0f;
		float ScaleRate = 0.0f;

		float Opacity	   = 0.0f;
		float OpacityStart = 0.0f;
	};

	extern std::array<BloodDrip, BLOOD_DRIP_NUM_MAX> BloodDrips;
	extern std::deque<BloodStain>					 BloodStains;

	void SpawnBloodMist(const Vector3i& pos, int roomNumber);
	void SpawnBloodMistCloud(const Vector3i& pos, int roomNumber);
	void SpawnBloodMistCloudUnderwater(const Vector3i& pos, int roomNumber);

	void SpawnBloodDrip(const Vector3& pos, int roomNumber, const Vector3& velocity, float scale);
	void SpawnBloodDripSpray(const Vector3& pos, int roomNumber, const Vector3& direction, unsigned int numDrips = BLOOD_DRIP_SPRAY_NUM_DEFAULT);

	void SpawnBloodStain(const Vector3& pos, int roomNumber, const Vector3& normal, float scaleMax, float scaleRate);

	void UpdateBloodMists();
	void UpdateBloodDrips();
	void UpdateBloodStains();

	void DrawIdioticPlaceholders();
}
