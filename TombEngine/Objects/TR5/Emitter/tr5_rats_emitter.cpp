#include "framework.h"
#include "Objects/TR5/Emitter/tr5_rats_emitter.h"
#include "Specific/level.h"
#include "Game/collision/collide_room.h"
#include "Game/control/flipeffect.h"
#include "Specific/setup.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Sound/sound.h"
#include "Game/Lara/lara.h"
#include "Game/items.h"

int NextRat;
RatData Rats[NUM_RATS];

short GetNextRat()
{
	short ratNumber = NextRat;
	auto* rat = &Rats[NextRat];

	int i = 0;
	while (rat->On)
	{
		if (ratNumber == NUM_RATS - 1)
		{
			rat = &Rats[0];
			ratNumber = 0;
		}
		else
		{
			ratNumber++;
			rat++;
		}

		i++;

		if (i >= NUM_RATS)
			return NO_ITEM;
	}

	NextRat = (ratNumber + 1) & 0x1F;
	return ratNumber;
}

void LittleRatsControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->TriggerFlags)
	{
		if (!item->ItemFlags[2] || !(GetRandomControl() & 0xF))
		{
			item->TriggerFlags--;

			if (item->ItemFlags[2] && GetRandomControl() & 1)
				item->ItemFlags[2]--;

			short ratNumber = GetNextRat();
			if (ratNumber != -1)
			{
				auto* rat = &Rats[ratNumber];

				rat->Pose.Position = item->Pose.Position;
				rat->RoomNumber = item->RoomNumber;

				if (item->ItemFlags[0])
				{
					rat->Pose.Orientation.SetY(2 * GetRandomControl()); // TODO: Short to float conversion.
					rat->VerticalVelocity = -16 - (GetRandomControl() & 31);
				}
				else
				{
					rat->VerticalVelocity = 0;
					rat->Pose.Orientation.y += Angle::ShrtToRad(GetRandomControl() & 0x3FFF) - Angle::DegToRad(45.0f);
				}

				rat->Pose.Orientation.x = 0.0f;
				rat->Pose.Orientation.z = 0.0f;
				rat->On = true;
				rat->Velocity = (GetRandomControl() & 31) + 1;
				rat->Flags = GetRandomControl() & 30;
			}
		}
	}
}

void ClearRats()
{
	if (Objects[ID_RATS_EMITTER].loaded)
	{
		ZeroMemory(Rats, NUM_RATS * sizeof(RatData));
		NextRat = 0;
		FlipEffect = -1;
	}
}

void InitialiseLittleRats(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	char flags = item->TriggerFlags / 1000;

	item->Pose.Orientation.x = Angle::DegToRad(45.0f);
	item->ItemFlags[1] = flags & 2;
	item->ItemFlags[2] = flags & 4;
	item->ItemFlags[0] = flags & 1;
	item->TriggerFlags = item->TriggerFlags % 1000;

	if (flags & 1)
	{
		ClearRats();
		return;
	}

	if (item->Pose.Orientation.y > Angle::DegToRad(-157.5f) && item->Pose.Orientation.y < Angle::DegToRad(-22.5f))
		item->Pose.Position.x += CLICK(2);
	else if (item->Pose.Orientation.y > Angle::DegToRad(22.5f) && item->Pose.Orientation.y < Angle::DegToRad(157.5f))
		item->Pose.Position.x -= CLICK(2);

	if (item->Pose.Orientation.y > Angle::DegToRad(-45.0f) && item->Pose.Orientation.y < Angle::DegToRad(45.0f))
		item->Pose.Position.z -= CLICK(2);
	else if (item->Pose.Orientation.y < Angle::DegToRad(-112.5f) || item->Pose.Orientation.y > Angle::DegToRad(112.5f))
		item->Pose.Position.z += CLICK(2);

	ClearRats();
}

void UpdateRats()
{
	if (Objects[ID_RATS_EMITTER].loaded)
	{
		for (int i = 0; i < NUM_RATS; i++)
		{
			auto* rat = &Rats[i];

			if (rat->On)
			{
				auto oldPos = rat->Pose.Position;

				rat->Pose.Position.x += rat->Velocity * sin(rat->Pose.Orientation.y);
				rat->Pose.Position.y += rat->VerticalVelocity;
				rat->Pose.Position.z += rat->Velocity * cos(rat->Pose.Orientation.y);

				rat->VerticalVelocity += GRAVITY;

				auto dPos = LaraItem->Pose.Position - rat->Pose.Position;

				float angle;
				if (rat->Flags >= 170)
					angle = rat->Pose.Orientation.y - atan2f(dPos.z, dPos.x);
				else
					angle = atan2(dPos.z, dPos.x) - rat->Pose.Orientation.y;

				if (abs(dPos.x) < 85 && abs(dPos.y) < 85 && abs(dPos.z) < 85)
				{
					LaraItem->HitPoints--;
					LaraItem->HitStatus = true;
				}

				// if life is even
				if (rat->Flags & 1)
				{
					// if rat is very near
					if (abs(dPos.z) + abs(dPos.x) <= SECTOR(1))
					{
						if (rat->Velocity & 1)
							rat->Pose.Orientation.SetY(rat->Pose.Orientation.y + Angle::DegToRad(2.8f));
						else
							rat->Pose.Orientation.SetY(rat->Pose.Orientation.y - Angle::DegToRad(2.8f));
						rat->Velocity = 48 - (abs(angle) / Angle::DegToRad(5.6f));
					}
					else
					{
						if (rat->Velocity < (i & 31) + 24)
							rat->Velocity++;

						if (abs(angle) >= Angle::DegToRad(11.25f))
						{
							if (angle >= 0)
								rat->Pose.Orientation.SetY(rat->Pose.Orientation.y + Angle::DegToRad(5.6f));
							else
								rat->Pose.Orientation.SetY(rat->Pose.Orientation.y - Angle::DegToRad(5.6f));
						}
						else
							rat->Pose.Orientation.y += Angle::ShrtToRad((Wibble - i) * 8);
					}
				}

				short oldRoomNumber = rat->RoomNumber;

				FloorInfo* floor = GetFloor(rat->Pose.Position.x, rat->Pose.Position.y, rat->Pose.Position.z,&rat->RoomNumber);
				int height = GetFloorHeight(floor, rat->Pose.Position.x, rat->Pose.Position.y, rat->Pose.Position.z);

				// if height is higher than 5 clicks 
				if (height < rat->Pose.Position.y - 1280 ||
					height == NO_HEIGHT)
				{
					// if timer is higher than 170 time to disappear 
					if (rat->Flags > 170)
					{
						rat->On = 0;
						NextRat = 0;
					}

					if (angle <= 0)
						rat->Pose.Orientation.SetY(rat->Pose.Orientation.y - Angle::DegToRad(90.0f));
					else
						rat->Pose.Orientation.SetY(rat->Pose.Orientation.y + Angle::DegToRad(90.0f));

					// reset rat to old Poseition and disable fall
					rat->Pose.Position = oldPos;
					rat->VerticalVelocity = 0;
				}
				else
				{
					// if height is lower than Y + 64
					if (height >= rat->Pose.Position.y - 64)
					{
						// if rat is higher than floor
						if (height >= rat->Pose.Position.y)
						{
							// if VerticalVelocity is too much or life is ended then kill rat
							if (rat->VerticalVelocity >= 500 ||
								rat->Flags >= 200)
							{
								rat->On = 0;
								NextRat = 0;
							}
							else
								rat->Pose.Orientation.x = Angle::ShrtToRad(-128 * rat->VerticalVelocity);
						}
						else
						{
							rat->Pose.Position.y = height;
							rat->VerticalVelocity = 0;
							rat->Flags |= 1;
						}
					}
					else
					{
						// if block is higher than rat Poseition then run vertically
						rat->Pose.Orientation.x = Angle::DegToRad(78.75f);
						rat->Pose.Position = oldPos;
						rat->Pose.Position.y -= 24;
						rat->VerticalVelocity = 0;
					}
				}

				if (!(Wibble & 60))
					rat->Flags += 2;

				auto* room = &g_Level.Rooms[rat->RoomNumber];

				if (TestEnvironment(ENV_FLAG_WATER, room->flags))
				{
					rat->Pose.Position.y = room->maxceiling + 50;
					rat->Velocity = 16;
					rat->VerticalVelocity = 0;

					if (TestEnvironment(ENV_FLAG_WATER, oldRoomNumber))
					{
						if (!(GetRandomControl() & 0xF))
							SetupRipple(rat->Pose.Position.x, room->maxceiling, rat->Pose.Position.z, (GetRandomControl() & 3) + 48, RIPPLE_FLAG_SHORT_INIT);
					}
					else
					{
						AddWaterSparks(rat->Pose.Position.x, room->maxceiling, rat->Pose.Position.z, 16);
						SetupRipple(rat->Pose.Position.x, room->maxceiling, rat->Pose.Position.z, (GetRandomControl() & 3) + 48, RIPPLE_FLAG_SHORT_INIT);
						SoundEffect(SFX_TR5_RATS_SPLASH,&rat->Pose);
					}
				}

				if (!i && !(GetRandomControl() & 4))
					SoundEffect(SFX_TR5_RATS,&rat->Pose);
			}
		}
	}
}
