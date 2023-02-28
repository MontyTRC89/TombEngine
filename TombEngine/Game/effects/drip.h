#pragma once

struct ItemInfo;

namespace TEN::Effects::Drip
{
	struct Drip
	{
		Vector3 Position   = Vector3::Zero;
		int		RoomNumber = 0;
		Vector3 Velocity   = Vector3::Zero;
		Vector2 Size	   = Vector2::Zero;
		Vector4 Color	   = Vector4::Zero;

		float Life	  = 0.0f;
		float LifeMax = 0.0f;
		float Gravity = 0.0f;
	};

	extern std::vector<Drip> Drips;

	void SpawnDrip(const Vector3& pos, int roomNumber, const Vector3& velocity, float lifeInSec, float gravity);
	void SpawnSplashDrips(const Vector3& pos, int roomNumber, unsigned int count, bool isSmallSplash = false);
	void SpawnWetnessDrip(const Vector3& pos, int roomNumber);

	void UpdateDrips();
	void ClearDrips();
}
