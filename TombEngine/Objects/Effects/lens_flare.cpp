#include "framework.h"
#include "Objects/Effects/lens_flare.h"
#include "Specific/level.h"
#include "Game/camera.h"
#include "Game/control/los.h"

namespace TEN::Entities::Effects
{
	std::vector<LensFlare> LensFlares;

	void LensFlareControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (TriggerActive(item))
		{
			SetupLensFlare(
				item->Pose.Position.ToVector3(),
				Vector3::One,
				item->RoomNumber, 
				false, 
				SPR_LENSFLARE3);
		}
	}

	void ClearLensFlares()
	{
		LensFlares.clear();
	}

	void SetupGlobalLensFlare(Vector2 yawAndPitchInDegrees, Vector3 color, int spriteIndex)
	{
		Vector3 position = Camera.pos.ToVector3();
		Matrix rotation = Matrix::CreateFromYawPitchRoll(
			DEG_TO_RAD(yawAndPitchInDegrees.x), 
			DEG_TO_RAD(yawAndPitchInDegrees.y), 
			0
		);
		position += Vector3::Transform(Vector3(0, 0, BLOCK(256)), rotation);
		SetupLensFlare(position, color, NO_VALUE, true, spriteIndex);
	}

	void SetupLensFlare(Vector3 position, Vector3 color, short roomNumber, bool global, int spriteIndex)
	{
		Vector3 lensFlarePosition;

		if (global)
		{
			if (g_Level.Rooms[Camera.pos.RoomNumber].flags & ENV_FLAG_NO_LENSFLARE)
			{
				return;
			}

			lensFlarePosition = position;
			Vector3 delta = (lensFlarePosition - Camera.pos.ToVector3()) / 16.0f;
			while (abs(delta.x) > BLOCK(200) || abs(delta.y) > BLOCK(200) || abs(delta.z) > BLOCK(200))
			{
				lensFlarePosition -= delta;
			}

			delta = (lensFlarePosition - Camera.pos.ToVector3()) / 16.0f;
			while (abs(delta.x) > BLOCK(32) || abs(delta.y) > BLOCK(32) || abs(delta.z) > BLOCK(32))
			{
				lensFlarePosition -= delta;
			}

			delta = (lensFlarePosition - Camera.pos.ToVector3()) / 16.0f;
			for (int i = 0; i < 16; i++)
			{
				short foundRoomNumber = IsRoomOutside(lensFlarePosition.x, lensFlarePosition.y, lensFlarePosition.z);
				if (foundRoomNumber != NO_VALUE)
				{
					roomNumber = foundRoomNumber;
					break;
				}
				lensFlarePosition -= delta;
			}
		}
		else
		{
			if (Vector3::Distance(position, Camera.pos.ToVector3()) > BLOCK(32))
			{
				return;
			}
			lensFlarePosition = position;
		}

		bool flareVisible = false;

		if (roomNumber != NO_VALUE)
		{
			if (g_Level.Rooms[roomNumber].flags & ENV_FLAG_NOT_NEAR_OUTSIDE || !global)
			{
				GameVector source = { Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.RoomNumber };
				GameVector destination = { (int)lensFlarePosition.x, (int)lensFlarePosition.y, (int)lensFlarePosition.z, roomNumber };
				flareVisible = LOS(&source, &destination);
			}
		}

		if (!flareVisible && !global)
		{
			return;
		}

		LensFlare lensFlare;

		lensFlare.Position = position;
		lensFlare.RoomNumber = roomNumber;
		lensFlare.Global = global;
		lensFlare.Color = color;
		lensFlare.SpriteIndex = spriteIndex;

		LensFlares.push_back(lensFlare);
	}
}