#include "framework.h"
#include "tr4_teethspike.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Specific/trmath.h"
#include "Objects/Utils/VehicleHelpers.h"

namespace TEN::Entities::TR4
{
	constexpr auto TEETH_SPIKES_DEFAULT_INTERVAL = 64;
	constexpr auto TEETH_SPIKE_HARM_CONSTANT = 8;
	constexpr auto TEETH_SPIKE_HARM_EMERGING = 30;

	void InitialiseTeethSpikes(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		item->Status = ITEM_INVISIBLE;

		// Set mutators to 0 by default
		for (int i = 0; i < item->Animation.Mutator.size(); i++)
			item->Animation.Mutator[i].Scale.y = 0.0f;

		item->ItemFlags[0] = 1024;
		item->ItemFlags[2] = 0;
	}

	bool TestBoundsCollideTeethSpikes(ItemInfo* item, ItemInfo* collidingItem)
	{
		// Get both teeth spikes and colliding item bounds
		auto spikeBox = TO_DX_BBOX(item->Pose, GetBoundsAccurate(item));
		auto itemBox = TO_DX_BBOX(collidingItem->Pose, GetBoundsAccurate(collidingItem));

		// Make intersection a bit more forgiving by reducing spike bounds a bit
		spikeBox.Extents = spikeBox.Extents * 0.95f;

		return spikeBox.Intersects(itemBox);
	}

	void ControlTeethSpikes(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (TriggerActive(item) && item->ItemFlags[2] == 0)
		{
			// Play sound if spikes are emerging. Ignore for constantly sticked out ones.
			if (item->ItemFlags[0] == 1024 && item->TriggerFlags != 1)
				SoundEffect(SFX_TR4_TEETH_SPIKES, &item->Pose);

			// Kill enemies
			item->Animation.Velocity.z = VEHICLE_COLLISION_TERMINAL_VELOCITY;
			DoVehicleCollision(item, CLICK(1.5f));

			item->Status = ITEM_ACTIVE;

			if (LaraItem->Animation.ActiveState != LS_DEATH && TestBoundsCollideTeethSpikes(item, LaraItem))
			{
				Vector3 normal = Vector3::UnitY;
				normal = normal.Transform(normal, Matrix::CreateFromYawPitchRoll(TO_RAD(item->Pose.Orientation.y), TO_RAD(item->Pose.Orientation.x), TO_RAD(item->Pose.Orientation.z)));
				auto dot = Vector3::UnitX.Dot(normal);
				auto angle = acos(dot / sqrt(normal.LengthSquared() * Vector3::UnitX.LengthSquared()));

				auto* bounds = (BOUNDING_BOX*)GetBestFrame(item);
				auto* laraBounds = (BOUNDING_BOX*)GetBestFrame(LaraItem);

				int bloodCount = 0;

				if ((item->ItemFlags[0] > 1024 || LaraItem->Animation.IsAirborne) &&
					(angle > PI * 0.25f && angle < PI * 0.75f))
				{
					if (LaraItem->Animation.Velocity.y > 6 || item->ItemFlags[0] > 1024)
					{
						LaraItem->HitPoints = -1;
						bloodCount = 20;
					}
				}
				else if ((item->TriggerFlags != 1) || LaraItem->Animation.Velocity.z >= 30)
				{
					int harm = item->ItemFlags[0] == 1024 ? TEETH_SPIKE_HARM_EMERGING : TEETH_SPIKE_HARM_CONSTANT;
					DoDamage(LaraItem, harm);
					bloodCount = (GetRandomControl() & 3) + 2;
				}
				else
					bloodCount = 0;

				int yTop = laraBounds->Y1 + LaraItem->Pose.Position.y;
				int yBottom = laraBounds->Y2 + LaraItem->Pose.Position.y;
				int y1, y2;

				if (angle < PI * 0.125f || angle > PI * 0.825f)
				{
					y1 = -bounds->Y2;
					y2 = -bounds->Y1;
				}
				else
				{
					y1 = bounds->Y1;
					y2 = bounds->Y2;
				}

				if (yTop < y1 + item->Pose.Position.y)
					yTop = y1 + item->Pose.Position.y;
				if (yBottom > y2 + item->Pose.Position.y)
					yBottom = y2 + item->Pose.Position.y;	

				int dy = (abs(yTop - yBottom)) + 1;

				if ((angle > PI * 0.125f && angle < PI * 0.375f) ||
					(angle > PI * 0.625f && angle < PI * 0.750f))
				{
					bloodCount >>= 1;
				}

				for (int i = 0; i < bloodCount; i++)
				{
					int dx = LaraItem->Pose.Position.x + (GetRandomControl() & 127) - 64;
					int dz = LaraItem->Pose.Position.z + (GetRandomControl() & 127) - 64;
					TriggerBlood(dx, yBottom - (GetRandomControl() % dy), dz, GetRandomControl() << 1, 1);
				}

				if (LaraItem->HitPoints <= 0 && Lara.Vehicle == NO_ITEM)
				{
					int heightFromFloor = GetCollision(LaraItem).Position.Floor - LaraItem->Pose.Position.y;
					if (item->Pose.Position.y >= LaraItem->Pose.Position.y &&
						heightFromFloor < CLICK(1))
					{
						SetAnimation(LaraItem, LA_SPIKE_DEATH);
						LaraItem->Animation.IsAirborne = false;
					}
				}
			}

			item->ItemFlags[0] += 128;
			item->ItemFlags[1] += item->ItemFlags[0];
			if (item->ItemFlags[1] >= 5120)
			{
				item->ItemFlags[1] = 5120;
				if (item->ItemFlags[0] <= 1024)
				{
					item->ItemFlags[0] = 0;
					if (item->TriggerFlags != 1 && LaraItem->HitPoints > 0)
					{
						int customInterval = item->TriggerFlags >> 2;
						item->ItemFlags[2] = customInterval ? customInterval : TEETH_SPIKES_DEFAULT_INTERVAL;
					}
				}
				else
					item->ItemFlags[0] = -item->ItemFlags[0] >> 1;
			}

		}
		else if (TriggerActive(item))
		{
			item->ItemFlags[0] += (item->ItemFlags[0] >> 3) + 32;
			item->ItemFlags[1] -= item->ItemFlags[0];
			if (item->ItemFlags[1] < 0)
			{
				item->ItemFlags[0] = 1024;
				item->ItemFlags[1] = 0;
				item->Status = ITEM_INVISIBLE;
			}

			if (item->TriggerFlags != 2)
			{
				if (item->ItemFlags[2])
					item->ItemFlags[2]--;
			}
			else
				item->ItemFlags[2] = 1;
		}
		else if (!item->Timer)
		{
			item->ItemFlags[0] += (item->ItemFlags[0] >> 3) + 32;
			if (item->ItemFlags[1] > 0)
			{
				item->ItemFlags[1] -= item->ItemFlags[0];
				if (item->ItemFlags[1] < 0)
					item->ItemFlags[1] = 0;
			}
		}

		// Update bone mutators.
		if (item->ItemFlags[1])
		{
			for (int i = 0; i < item->Animation.Mutator.size(); i++)
				item->Animation.Mutator[i].Scale = Vector3(1.0f, item->ItemFlags[1] / 4096.0f, 1.0f);
		}
	}
}
