#include "framework.h"
#include "Objects/Effects/LensFlare.h"

#include "Game/camera.h"
#include "Game/control/los.h"
#include "Math/Math.h"
#include "Scripting/Include/ScriptInterfaceLevel.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/level.h"

using namespace TEN::Math;

namespace TEN::Entities::Effects
{
	std::vector<LensFlare> LensFlares;

	static void SetupLensFlare(const Vector3& pos, int roomNumber, const Color& color, bool isGlobal, int spriteID)
	{
		auto lensFlarePos = Vector3::Zero;
		if (isGlobal)
		{
			if (TestEnvironment(ENV_FLAG_NO_LENSFLARE, Camera.pos.RoomNumber))
				return;

			lensFlarePos = pos;
			auto delta = (lensFlarePos - Camera.pos.ToVector3()) / 16.0f;
			while (abs(delta.x) > BLOCK(200) || abs(delta.y) > BLOCK(200) || abs(delta.z) > BLOCK(200))
				lensFlarePos -= delta;

			delta = (lensFlarePos - Camera.pos.ToVector3()) / 16.0f;
			while (abs(delta.x) > BLOCK(32) || abs(delta.y) > BLOCK(32) || abs(delta.z) > BLOCK(32))
				lensFlarePos -= delta;

			delta = (lensFlarePos - Camera.pos.ToVector3()) / 16.0f;
			for (int i = 0; i < 16; i++)
			{
				int foundRoomNumber = IsRoomOutside(lensFlarePos.x, lensFlarePos.y, lensFlarePos.z);
				if (foundRoomNumber != NO_VALUE)
				{
					roomNumber = foundRoomNumber;
					break;
				}

				lensFlarePos -= delta;
			}
		}
		else
		{
			float dist = Vector3::Distance(pos, Camera.pos.ToVector3());
			if (dist > BLOCK(32))
				return;

			lensFlarePos = pos;
		}

		bool isVisible = false;
		if (roomNumber != NO_VALUE)
		{
			if (TestEnvironment(ENV_FLAG_NOT_NEAR_OUTSIDE, roomNumber) || !isGlobal)
			{
				auto origin = Camera.pos;
				auto target = GameVector(lensFlarePos, roomNumber);
				isVisible = LOS(&origin, &target);
			}
		}

		if (!isVisible && !isGlobal)
			return;

		auto lensFlare = LensFlare{};
		lensFlare.Position = pos;
		lensFlare.RoomNumber = roomNumber;
		lensFlare.IsGlobal = isGlobal;
		lensFlare.Color = color;
		lensFlare.SpriteID = spriteID;

		LensFlares.push_back(lensFlare);
	}

	void UpdateGlobalLensFlare()
	{
		constexpr auto BASE_POS = Vector3(0.0f, 0.0f, BLOCK(256));

		if (!g_GameFlow->GetLevel(CurrentLevel)->GetLensFlareEnabled())
			return;

		auto orient   = EulerAngles(g_GameFlow->GetLevel(CurrentLevel)->GetLensFlarePitch(), g_GameFlow->GetLevel(CurrentLevel)->GetLensFlareYaw(), 0);
		auto color    = g_GameFlow->GetLevel(CurrentLevel)->GetLensFlareColor();
		auto spriteID = g_GameFlow->GetLevel(CurrentLevel)->GetLensFlareSunSpriteID();
		
		auto pos = Camera.pos.ToVector3();
		auto rotMatrix = orient.ToRotationMatrix();

		pos += Vector3::Transform(BASE_POS, rotMatrix);
		SetupLensFlare(pos, NO_VALUE, color, true, spriteID);
	}

	void ControlLensFlare(int itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (TriggerActive(&item))
			SetupLensFlare(item.Pose.Position.ToVector3(), item.RoomNumber, item.Model.Color, false, SPRITE_TYPES::SPR_LENS_FLARE_3);
	}

	void ClearLensFlares()
	{
		LensFlares.clear();
	}
}
