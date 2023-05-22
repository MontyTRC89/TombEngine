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
		Vector2 Size	   = Vector2::Zero;
		Vector4 Color	   = Vector4::Zero;

		float Life			  = 0.0f;
		float LifeStartFading = 0.0f;
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

	// TODO: Copy approach from ripple effect.
	struct UnderwaterBloodEffectParticle
	{
		unsigned int SpriteID = 0;

		Vector3 Position   = Vector3::Zero;
		int		RoomNumber = 0;
		Vector4 Color	   = Vector4::Zero;

		float Life	  = 0.0f;
		float Init	  = 0.0f;
		float Size	  = 0.0f;
		float Opacity = 0.0f;

		void Update();
	};

	class UnderwaterBloodEffectController
	{
	private:
		// Members
		std::vector<UnderwaterBloodEffectParticle> Particles;

	public:
		// Getters
		const std::vector<UnderwaterBloodEffectParticle>& GetParticles();

		// Spawners
		void Spawn(const Vector3& pos, int roomNumber, float size, unsigned int count = 1);

		// Utilities
		void Update();
		void Clear();
	};

	extern UnderwaterBloodEffectController UnderwaterBlood;

	extern std::vector<BloodDrip>		BloodDrips;
	extern std::vector<BloodStain>		BloodStains;
	extern std::vector<BloodMist>		BloodMists;

	void SpawnBloodSplat(const Vector3& pos, int roomNumber, const Vector3& di, const Vector3& baseVel, unsigned int baseCount);

	void SpawnBloodDrip(const Vector3& pos, int roomNumber, const Vector3& vel, const Vector2& siz, float lifeInSec, bool canSpawnStain);
	void SpawnBloodStain(const Vector3& pos, int roomNumber, const Vector3& normal, float scaleMax, float scaleRate, float delayInSec = 0.0f);
	void SpawnBloodStain(const BloodDrip& drip, const CollisionResult& pointColl, bool isOnFloor);
	void SpawnBloodStain(ItemInfo& item);
	void SpawnBloodMist(const Vector3& pos, int roomNumber, const Vector3& dir);
	void SpawnBloodMists(const Vector3& pos, int roomNumber, const Vector3& dir, unsigned int count);

	void UpdateBloodDrips();
	void UpdateBloodStains();
	void UpdateBloodMists();

	void ClearBloodDrips();
	void ClearBloodStains();
	void ClearBloodMists();

	void DrawBloodDebug();
}
