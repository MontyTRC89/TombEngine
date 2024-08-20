#pragma once

#include "Objects/game_object_ids.h"

// TODO: forward decl
#include "Game/collision/Point.h"
using namespace TEN::Collision::Point;

//namespace TEN::Collision::Point { class PointCollisionData; }
struct ItemInfo;

namespace TEN::Effects::Blood
{
	struct BloodDripEffectParticle
	{
		static constexpr auto LIFE_START_FADING = 0.5f;

		GAME_OBJECT_ID SpriteSeqID = GAME_OBJECT_ID::ID_DEFAULT_SPRITES;
		int			   SpriteID	   = 0;

		Vector3 Position   = Vector3::Zero;
		int		RoomNumber = 0;
		Vector3 Velocity   = Vector3::Zero;
		Vector4 Color	   = Vector4::Zero;

		float Life			  = 0.0f;
		float LifeStartFading = 0.0f;
		float Size			  = 0.0f;
		float Opacity		  = 0.0f;
		float Gravity		  = 0.0f;

		void Update();
	};

	class BloodDripEffectController
	{
	private:
		// Members

		std::vector<BloodDripEffectParticle> _particles;

	public:
		// Getters

		const std::vector<BloodDripEffectParticle>& GetParticles();

		// Spawners

		void Spawn(const Vector3& pos, int roomNumber, const Vector3& vel, float size, float lifeInSec);

		// Utilities

		void Update();
		void Clear();
	};

	struct BloodStainEffectParticle
	{
		static constexpr auto LIFE_MAX				   = 5.0f * 60.0f;
		static constexpr auto LIFE_START_FADING		   = 15.0f;
		static constexpr auto SURFACE_OFFSET		   = 4;
		static constexpr auto VERTEX_COUNT			   = 4;
		static constexpr auto COLL_CHECK_TIME_INTERVAL = 0.5f;

		GAME_OBJECT_ID SpriteSeqID = GAME_OBJECT_ID::ID_DEFAULT_SPRITES;
		int			   SpriteID	   = 0;

		Vector3	Position	= Vector3::Zero;
		int		RoomNumber	= 0;
		short	Orientation = 0;
		Vector3 Normal		= Vector3::Zero;
		Vector4	Color		= Vector4::Zero;
		Vector4	ColorStart	= Vector4::Zero;
		Vector4	ColorEnd	= Vector4::Zero;

		std::array<Vector3, VERTEX_COUNT> Vertices = {};

		float Life			  = 0.0f;
		float LifeStartFading = 0.0f;
		float Size			  = 0.0f;
		float SizeMax		  = 0.0f;
		float Scalar		  = 0.0f;
		float Opacity		  = 0.0f;
		float OpacityMax	  = 0.0f;
		float DelayTime		  = 0.0f;

		bool  IsOnFloor			  = false;
		float CollCheckTimeOffset = 0.0f;

		void Update();

		std::array<Vector3, VERTEX_COUNT> GetVertices();
		bool							  TestSurface() const;
	};

	class BloodStainEffectController
	{
	private:
		// Members

		std::vector<BloodStainEffectParticle> _particles = {};

	public:
		// Getters

		const std::vector<BloodStainEffectParticle>& GetParticles();

		// Spawners

		void Spawn(const Vector3& pos, int roomNumber, const Vector3& normal, float sizeMax, float scalar, float delayInSec = 0.0f);
		void Spawn(const BloodDripEffectParticle& drip, PointCollisionData& pointColl, bool isOnFloor);
		void Spawn(const ItemInfo& item);

		// Utilities

		void Update();
		void Clear();
	};

	struct BloodBillboardEffectParticle
	{
		GAME_OBJECT_ID SpriteSeqID = GAME_OBJECT_ID::ID_DEFAULT_SPRITES;
		int			   SpriteID	   = 0;

		Vector3 Position   = Vector3::Zero;
		int		RoomNumber = 0;
		Vector4	Color	   = Vector4::Zero;

		float Life = 0.0f;
		float Size = 0.0f;

		void Update();
	};

	class BloodBillboardEffectController
	{
	private:
		// Members

		std::vector<BloodBillboardEffectParticle> _particles = {};

	public:
		// Getters

		const std::vector<BloodBillboardEffectParticle>& GetParticles();

		// Spawners

		void Spawn(const Vector3& pos, int roomNumber, float size);

		// Utilities

		void Update();
		void Clear();
	};
	
	struct BloodMistEffectParticle
	{
		GAME_OBJECT_ID SpriteSeqID = GAME_OBJECT_ID::ID_DEFAULT_SPRITES;
		int			   SpriteID	   = 0;

		Vector3 Position	  = Vector3::Zero;
		int		RoomNumber	  = 0;
		short	Orientation2D = 0;
		Vector3 Velocity	  = Vector3::Zero;
		Vector4 Color		  = Vector4::Zero;
		
		float Life		 = 0.0f;
		float LifeMax	 = 0.0f;
		float Size		 = 0.0f;
		float SizeMax	 = 0.0f;
		float SizeMin	 = 0.0f;
		float Opacity	 = 0.0f;
		float OpacityMax = 0.0f;
		float Gravity	 = 0.0f;
		float Friction	 = 0.0f;
		short Rotation	 = 0;

		void Update();
	};

	class BloodMistEffectController
	{
	private:
		// Members

		std::vector<BloodMistEffectParticle> _particles = {};

	public:
		// Getters

		const std::vector<BloodMistEffectParticle>& GetParticles();

		// Spawners

		void Spawn(const Vector3& pos, int roomNumber, const Vector3& dir, unsigned int count = 1);

		// Utilities

		void Update();
		void Clear();
	};

	// TODO: Copy approach from ripple effect.
	struct UnderwaterBloodCloudEffectParticle
	{
		GAME_OBJECT_ID SpriteSeqID = GAME_OBJECT_ID::ID_DEFAULT_SPRITES;
		int			   SpriteID	   = 0;

		Vector3 Position   = Vector3::Zero;
		int		RoomNumber = 0;
		Vector4 Color	   = Vector4::Zero;

		float Life	  = 0.0f;
		float Init	  = 0.0f;
		float Size	  = 0.0f;
		float Opacity = 0.0f;

		void Update();
	};

	class UnderwaterBloodCloudEffectController
	{
	private:
		// Members

		std::vector<UnderwaterBloodCloudEffectParticle> _particles;

	public:
		// Getters

		const std::vector<UnderwaterBloodCloudEffectParticle>& GetParticles();

		// Spawners

		void Spawn(const Vector3& pos, int roomNumber, float size, unsigned int count = 1);

		// Utilities

		void Update();
		void Clear();
	};

	extern BloodDripEffectController			BloodDripEffect;
	extern BloodStainEffectController			BloodStainEffect;
	extern BloodBillboardEffectController		BloodBillboardEffect;
	extern BloodMistEffectController			BloodMistEffect;
	extern UnderwaterBloodCloudEffectController UnderwaterBloodCloudEffect;

	void SpawnBloodSplatEffect(const Vector3& pos, int roomNumber, const Vector3& dir, const Vector3& baseVel, unsigned int count);
	void SpawnPlayerBloodEffect(const ItemInfo& item);

	// TODO: Remove legacy spawners

	void TriggerBlood(const Vector3& pos, short headingAngle, unsigned int count);
	short DoBloodSplat(int x, int y, int z, short vel, short yRot, short roomNumber);
	void DoLotsOfBlood(const Vector3& pos, int vel, short headingAngle, int roomNumber, unsigned int count);

	void DrawBloodDebug();
}
