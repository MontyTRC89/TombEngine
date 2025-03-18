#pragma once

#include "Objects/game_object_ids.h"
#include "Objects/objectslist.h"
#include "Math/Math.h"

struct ItemInfo;

using namespace TEN::Math;

namespace TEN::Effects::Fireflies
{
	enum class FirefliesItemFlags
	{
		TargetItemPtr,
		Light,
		TriggerFlags,
		SpawnCounter,
		RemoveFliesEffect,
		LightID0,
		LightID1
	};

	struct FireflyData
	{
		int		  SpriteSeqID = GAME_OBJECT_ID::ID_DEFAULT_SPRITES;
		int		  SpriteID	  = SPRITE_TYPES::SPR_UNDERWATER_DUST;
		BlendMode BlendMode	  = BlendMode::Additive;

		Vector3		Position	   = Vector3::Zero;
		int			RoomNumber	   = 0;
		Vector3		PositionTarget = Vector3::Zero;
		EulerAngles Orientation	   = EulerAngles::Identity;
		float		Velocity	   = 0.0f;
		Vector4		Color		   = Vector4::Zero; // TODO

		int	  ID   = 0;
		float Life = 0.0f;
		float Size;
		float zVel = 0.0f;
		
		ItemInfo* TargetItem = nullptr; // TODO: Use moveable ID instead.

		Vector3 PrevPosition = Vector3::Zero;
		Vector4 PrevColor	 = Vector4::Zero; // TODO

		// TODO: Not needed.
		unsigned char rB;
		unsigned char gB;
		unsigned char bB;
		unsigned char r;
		unsigned char g;
		unsigned char b;
		byte PrevR;
		byte PrevG;
		byte PrevB;

		void StoreInterpolationData()
		{
			PrevPosition = Position;
			PrevColor = Color;

			PrevR = r;
			PrevG = g;
			PrevB = b;
		}
	};

	extern std::vector<FireflyData> FireflySwarm;

	void InitializeFireflySwarm(short itemNumber);
	void ControlFireflySwarm(short itemNumber);
	void UpdateFireflySwarm();
	void ClearInactiveFireflies(ItemInfo& item);
	void ClearFireflySwarm();

	void SpawnFireflySwarm(ItemInfo& item, int triggerFlags);
}
