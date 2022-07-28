#include "framework.h"
#include "Objects/Effects/tr4_locusts.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/trmath.h"

namespace TEN::Entities::TR4 
{
	LOCUST_INFO Locusts[MAX_LOCUSTS];

	int CreateLocust(void)
	{
		for (int i = 0; i < MAX_LOCUSTS; i++)
		{
			auto* locust = &Locusts[i];
			if (!locust->on)
				return i;
		}

		return NO_ITEM;
	}

	void SpawnLocust(ItemInfo* item)
	{
		Vector3Int start, end;
	   short locustNumber = CreateLocust();
	   Vector3Shrt angles;
		if (locustNumber != NO_ITEM)
		{
			auto* locust = &Locusts[locustNumber];

			// Emitter.
			if (item->ObjectNumber == ID_LOCUSTS_EMITTER)
			{
				end = item->Pose.Position;
				angles = Vector3Shrt(
					0,
					item->Pose.Orientation.y - ANGLE(180.0f),
					0
				);
			}
			// Mutant.
			else
			{
				start.x = 0;
				start.y = -96;
				start.z = 144;
				GetJointAbsPosition(item, &start, 9);
				end.x = 0;
				end.y = -128;
				end.z = 288;
				GetJointAbsPosition(item, &end, 9);
				angles = GetVectorAngles(end.x - start.x, end.y - start.y, end.z - start.z);
			}

			// NOTE: this is not present in original TR4 code
			//target = GetCreatureInfo(item)->enemy;

			locust->on = true;
			//locust->target = target != nullptr ? target : nullptr;
			locust->pos.Position = end;
			locust->pos.Orientation.x = (GetRandomControl() & 0x3FF) + angles.x - ANGLE(2.8f);
			locust->pos.Orientation.y = (GetRandomControl() & 0x7FF) + angles.y - ANGLE(5.6f);
			locust->roomNumber = item->RoomNumber;
			locust->randomRotation = (GetRandomControl() & 0x1F) + 0x10;
			locust->escapeYrot = (GetRandomControl() & 0x1FF);
			locust->escapeXrot = ((GetRandomControl() & 0x7) + 0xF) * 20;
			locust->counter = 0;
		}
	}

	void InitialiseLocust(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->Pose.Orientation.y > 0)
		{
			if (item->Pose.Orientation.y == ANGLE(90.0f))
				item->Pose.Position.x += CLICK(2);
		}
		else if (item->Pose.Orientation.y < 0)
		{
			if (item->Pose.Orientation.y == -ANGLE(180.0f))
				item->Pose.Position.z -= CLICK(2);
			else if (item->Pose.Orientation.y == -ANGLE(90.0f))
				item->Pose.Position.x -= CLICK(2);
		}
		else
			item->Pose.Position.z += CLICK(2);
	}

	void LocustControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (TriggerActive(item))
		{
			if (item->TriggerFlags)
			{
				SpawnLocust(item);
				item->TriggerFlags--;
			}
			else
				KillItem(itemNumber);
		}
	}

	void UpdateLocusts(void)
	{
		for (int i = 0; i < MAX_LOCUSTS; i++)
		{
			auto* locust = &Locusts[i];
			if (locust->on)
			{
				// NOTE: not present in original TR4 code
				//if (LaraItem == nullptr)
				//    LaraItem = LaraItem;

				if ((Lara.Control.KeepLow || LaraItem->HitPoints <= 0) &&
					locust->counter >= 90 &&
					!(GetRandomControl() & 7))
				{
					locust->counter = 90;
				}

				locust->counter--;
				if (locust->counter == 0)
				{
					locust->on = false;
					break;
				}

				if (!(GetRandomControl() & 7))
				{
					locust->escapeYrot = GetRandomControl() % 640 + 128;
					locust->escapeXrot = (GetRandomControl() & 0x7F) - 64;
					locust->escapeZrot = (GetRandomControl() & 0x7F) - 64;
				}

				auto angles = GetVectorAngles(
					LaraItem->Pose.Position.x + 8 * locust->escapeXrot - locust->pos.Position.x,
					LaraItem->Pose.Position.y - locust->escapeYrot - locust->pos.Position.y,
					LaraItem->Pose.Position.z + 8 * locust->escapeZrot - locust->pos.Position.z);

				int distance = pow(LaraItem->Pose.Position.z - locust->pos.Position.z, 2) + pow(LaraItem->Pose.Position.x - locust->pos.Position.x, 2);
				int square = int(sqrt(distance)) / 8;
				if (square <= 128)
				{
					if (square < 48)
						square = 48;
				}
				else
					square = 128;

				if (locust->randomRotation < square)
					locust->randomRotation += 1;
				else if (locust->randomRotation > square)
					locust->randomRotation -= 1;

				if (locust->counter > 90)
				{
					short resultYrot, resultXrot;
					int shiftYrot, shiftXrot;
					int random = locust->randomRotation * 128;
					resultYrot = angles.y - locust->pos.Orientation.y;

					if (abs(resultYrot) > ANGLE(180.0f))
						resultYrot = locust->pos.Orientation.y - angles.y;

					resultXrot = angles.x - locust->pos.Orientation.x;

					if (abs(resultXrot) > ANGLE(180.0f))
						resultXrot = locust->pos.Orientation.x - angles.y;

					shiftXrot = resultXrot / 8;
					shiftYrot = resultYrot / 8;

					if (shiftYrot > random || shiftYrot < -random)
						shiftYrot = -random;

					if (shiftXrot > random || shiftXrot < -random)
						shiftXrot = -random;

					locust->pos.Orientation.x += shiftXrot;
					locust->pos.Orientation.y += shiftYrot;
				}

				locust->pos.Position.x += locust->randomRotation * phd_cos(locust->pos.Orientation.x) * phd_sin(locust->pos.Orientation.y);
				locust->pos.Position.y += locust->randomRotation * phd_sin(-locust->pos.Orientation.x);
				locust->pos.Position.z += locust->randomRotation * phd_cos(locust->pos.Orientation.x) * phd_cos(locust->pos.Orientation.y);
				
				if (ItemNearTarget(&locust->pos, LaraItem, CLICK(1) / 2))
				{
					TriggerBlood(locust->pos.Position.x, locust->pos.Position.y, locust->pos.Position.z, 2 * GetRandomControl(), 2);
					DoDamage(LaraItem, LOCUST_LARA_DAMAGE);
				}

				if (locust->counter > 0)
					SoundEffect(SFX_TR4_LOCUSTS_LOOP, &locust->pos);
			}
		}
	}

	void DrawLocust(void)
	{
		// TODO: no render for the locusts !
	}
}
