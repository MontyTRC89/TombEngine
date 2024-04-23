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
			SetupLensFlare(item->Pose.Position.ToVector3(), item->RoomNumber, false);
		}
	}

	void ClearLensFlares()
	{
		LensFlares.clear();
	}

	void SetupGlobalLensFlare(float yaw, float pitch)
	{
		Vector3 position = Camera.pos.ToVector3();
		Matrix rotation = Matrix::CreateFromYawPitchRoll(DEG_TO_RAD(yaw), DEG_TO_RAD(pitch), 0);
		position += Vector3::Transform(Vector3(0, 0, BLOCK(256)), rotation);
		SetupLensFlare(position, NO_VALUE, true);
	}

	void SetupLensFlare(Vector3 position, short roomNumber, bool sun)
	{
		Vector3 lensFlarePosition;

		if (sun)
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
			if (g_Level.Rooms[roomNumber].flags & ENV_FLAG_NOT_NEAR_OUTSIDE || !sun)
			{
				GameVector source = { Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.RoomNumber };
				GameVector destination = { (int)lensFlarePosition.x, (int)lensFlarePosition.y, (int)lensFlarePosition.z, roomNumber };
				flareVisible = LOS(&source, &destination);
			}
		}

		if (!flareVisible && !sun)
		{
			return;
		}

		LensFlare lensFlare;
		lensFlare.Position = position;
		lensFlare.RoomNumber = roomNumber;
		lensFlare.Sun = sun;
		LensFlares.push_back(lensFlare);
	}
}