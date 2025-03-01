#pragma once
#include "Game/items.h"
#include "Math/Math.h"

using namespace TEN::Math;

namespace TEN::Effects::Fireflys
{
	constexpr auto FISH_COUNT_MAX = 512;

	struct FireflyData
	{
		int	 MeshIndex	  = 0;
		bool IsPatrolling = false;
		bool IsLethal	  = false;
		int SpriteSeqID = ID_DEFAULT_SPRITES;
		int SpriteID = SPR_UNDERWATERDUST;
		unsigned int scalar;


		Vector3		Position	   = Vector3::Zero;
		int			RoomNumber	   = 0;
		Vector3		PositionTarget = Vector3::Zero;
		EulerAngles Orientation	   = EulerAngles::Identity;
		float		Velocity	   = 0.0f;

		float Life		 = 0.0f;
		float Undulation = 0.0f;

		ItemInfo* TargetItemPtr = nullptr;
		ItemInfo* LeaderItemPtr = nullptr;

		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char colFadeSpeed;
		unsigned char fadeToBlack;

		int x;
		int y;
		int z;

		bool on;
		int sLife;
		int life;

		float sSize;
		float dSize;
		float size;

		short rotAng;

		int PrevX;
		int PrevY;
		int PrevZ;
	
		byte PrevR;
		byte PrevG;
		byte PrevB;
		byte PrevScalar;

		BlendMode blendMode;

		void StoreInterpolationData()
		{
			PrevX = x;
			PrevY = y;
			PrevZ = z;			
		}
	};

	extern std::vector<FireflyData> FireflySwarm;

	void InitializeFireflySwarm(short itemNumber);
	void ControlFireflySwarm(short itemNumber);

	void UpdateFireflySwarm();
	void ClearFireflySwarm();
}
