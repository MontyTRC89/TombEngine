#pragma once

struct CollisionResult;
struct ItemInfo;

namespace TEN::Effects::Blood
{
	struct BloodDrip
	{
		static constexpr auto LIFE_START_FADING = 0.5f;

		unsigned int SpriteID	   = 0;
		bool		 CanSpawnStain = false;

		Vector3 Position   = Vector3::Zero;
		int		RoomNumber = 0;
		Vector3 Velocity   = Vector3::Zero;
		Vector4 Color	   = Vector4::Zero;

		float Life			  = 0.0f;
		float LifeStartFading = 0.0f;
		float Scale			  = 0.0f;
		float Opacity		  = 0.0f;
		float Gravity		  = 0.0f;
	};

	struct BloodStain
	{
		static constexpr auto LIFE_MAX			= 5.0f * 60.0f;
		static constexpr auto LIFE_START_FADING = 30.0f;
		static constexpr auto SURFACE_OFFSET	= 4;
		static constexpr auto VERTEX_COUNT		= 4;

		unsigned int SpriteID = 0;

		Vector3	Position	  = Vector3::Zero;
		int		RoomNumber	  = 0;
		short	Orientation2D = 0;
		Vector3 Normal		  = Vector3::Zero;
		Vector4	Color		  = Vector4::Zero;
		Vector4	ColorStart	  = Vector4::Zero;
		Vector4	ColorEnd	  = Vector4::Zero;

		std::array<Vector3, VERTEX_COUNT> VertexPoints = {};

		float Life			  = 0.0f;
		float LifeStartFading = 0.0f;
		float Scale			  = 0.0f;
		float ScaleMax		  = 0.0f;
		float ScaleRate		  = 0.0f;
		float Opacity		  = 0.0f;
		float OpacityMax	  = 0.0f;
		float DelayTime		  = 0.0f;
	};
	
	struct BloodMist
	{
		unsigned int SpriteID = 0;

		Vector3 Position	  = Vector3::Zero;
		int		RoomNumber	  = 0;
		short	Orientation2D = 0;
		Vector3 Velocity	  = Vector3::Zero;
		Vector4 Color		  = Vector4::Zero;
		
		float Life		 = 0.0f;
		float LifeMax	 = 0.0f;
		float Scale		 = 0.0f;
		float ScaleMax	 = 0.0f;
		float ScaleMin	 = 0.0f;
		float Opacity	 = 0.0f;
		float OpacityMax = 0.0f;
		float Gravity	 = 0.0f;
		float Friction	 = 0.0f;
		short Rotation	 = 0;
	};

	struct UnderwaterBlood
	{
		unsigned int SpriteID = 0;

		Vector3 Position   = Vector3::Zero;
		int		RoomNumber = 0;
		Vector4 Color	   = Vector4::Zero;

		float Life	  = 0.0f;
		float Init	  = 0.0f;
		float Size	  = 0.0f;
		float Opacity = 0.0f;
	};

	extern std::vector<BloodDrip>		BloodDrips;
	extern std::vector<BloodStain>		BloodStains;
	extern std::vector<BloodMist>		BloodMists;
	extern std::vector<UnderwaterBlood> UnderwaterBloodClouds;

	void SpawnBloodDrip(const Vector3& pos, int roomNumber, const Vector3& velocity, float lifeInSec, float scale, bool canSpawnStain);
	void SpawnBloodDripSpray(const Vector3& pos, int roomNumber, const Vector3& direction, const Vector3& baseVelocity, unsigned int count);
	void SpawnBloodStain(const Vector3& pos, int roomNumber, const Vector3& normal, float scaleMax, float scaleRate, float delayInSec = 0.0f);
	void SpawnBloodStainFromDrip(const BloodDrip& drip, const CollisionResult& pointColl);
	void SpawnBloodStainPool(ItemInfo& item);
	void SpawnBloodMist(const Vector3& pos, int roomNumber, const Vector3& direction);
	void SpawnBloodMistCloud(const Vector3& pos, int roomNumber, const Vector3& direction, unsigned int count);
	void SpawnUnderwaterBlood(const Vector3& pos, int roomNumber, float size);
	void SpawnUnderwaterBloodGroup(const Vector3& pos, int roomNumber, float sizeMax, unsigned int count);

	void UpdateBloodDrips();
	void UpdateBloodStains();
	void UpdateBloodMists();
	void UpdateUnderwaterBloodClouds();

	void ClearBloodDrips();
	void ClearBloodStains();
	void ClearBloodMists();
	void ClearUnderwaterBloodClouds();

	void DrawBloodDebug();
}
