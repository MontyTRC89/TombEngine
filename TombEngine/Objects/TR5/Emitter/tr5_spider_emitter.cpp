#include "framework.h"
#include "Objects/TR5/Emitter/tr5_spider_emitter.h"
#include "Specific/level.h"
#include "Game/collision/collide_room.h"
#include "Game/control/flipeffect.h"
#include "Specific/setup.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Sound/sound.h"
#include "Game/Lara/lara.h"
#include "Game/items.h"

int NextSpider;
SpiderData Spiders[NUM_SPIDERS];

short GetNextSpider()
{
	short spiderNumber = NextSpider;
	auto* spider = &Spiders[NextSpider];

	int i = 0;
	while (spider->On)
	{
		if (spiderNumber == (NUM_SPIDERS - 1))
		{
			spider = &Spiders[0];
			spiderNumber = 0;
		}
		else
		{
			++spiderNumber;
			++spider;
		}

		if (++i >= NUM_SPIDERS)
			return NO_ITEM;
	}

	NextSpider = (spiderNumber + 1) & (NUM_SPIDERS - 1);
	return spiderNumber;
}

void ClearSpiders()
{
	if (Objects[ID_SPIDERS_EMITTER].loaded)
	{
		ZeroMemory(Spiders, NUM_SPIDERS * sizeof(SpiderData));
		NextSpider = 0;
		FlipEffect = -1;
	}
}

void InitialiseSpiders(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	short flags = item->TriggerFlags / -24;

	item->Pose.Orientation.x = Angle::DegToRad(45.0f);
	item->ItemFlags[1] = flags & 2;
	item->ItemFlags[2] = flags & 4;
	item->ItemFlags[0] = flags & 1;
	item->TriggerFlags = flags % 1000;

	if (flags & 1)
	{
		ClearSpiders();
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

	ClearSpiders();
}

void SpidersEmitterControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->TriggerFlags)
	{
		if (!item->ItemFlags[2] || !(GetRandomControl() & 0xF))
		{
			item->TriggerFlags--;

			if (item->ItemFlags[2] && GetRandomControl() & 1)
				item->ItemFlags[2]--;

			short spiderNumber = GetNextSpider();
			if (spiderNumber != NO_ITEM)
			{
				auto* spider = &Spiders[spiderNumber];

				spider->Pose.Position = item->Pose.Position;
				spider->RoomNumber = item->RoomNumber;

				if (item->ItemFlags[0])
				{
					spider->Pose.Orientation.SetY(2 * GetRandomControl());
					spider->VerticalVelocity = -16 - (GetRandomControl() & 0x1F);
				}
				else
				{
					spider->Pose.Orientation.y += Angle::ShrtToRad(GetRandomControl() & 0x3FFF) - Angle::DegToRad(45.0f);
					spider->VerticalVelocity = 0;
				}

				spider->Pose.Orientation.x = 0.0f;
				spider->Pose.Orientation.z = 0.0f;
				spider->On = true;
				spider->Flags = 0;
				spider->Velocity = (GetRandomControl() & 0x1F) + 1;
			}
		}
	}
}

void UpdateSpiders()
{
	if (Objects[ID_SPIDERS_EMITTER].loaded)
	{
		for (int i = 0; i < NUM_SPIDERS; i++)
		{
			auto* spider = &Spiders[i];

			if (spider->On)
			{
				auto oldPos = spider->Pose.Position;

				spider->Pose.Position.x += spider->Velocity * sin(spider->Pose.Orientation.y);
				spider->Pose.Position.y += spider->VerticalVelocity;
				spider->Pose.Position.z += spider->Velocity * cos(spider->Pose.Orientation.y);
				spider->VerticalVelocity += GRAVITY;

				auto dPos = LaraItem->Pose.Position - spider->Pose.Position;

				float angle = atan2(dPos.z, dPos.x) - spider->Pose.Orientation.y;

				if (abs(dPos.x) < 85 && abs(dPos.y) < 85 && abs(dPos.z) < 85)
				{
					DoDamage(LaraItem, 3);
					TriggerBlood(spider->Pose.Position.x, spider->Pose.Position.y, spider->Pose.Position.z, spider->Pose.Orientation.y, 1);
				}

				if (spider->Flags)
				{
					if (abs(dPos.x) + abs(dPos.z) <= CLICK(3))
					{
						if (spider->Velocity & 1)
							spider->Pose.Orientation.SetY(spider->Pose.Orientation.y + Angle::DegToRad(2.8f));
						else
							spider->Pose.Orientation.SetY(spider->Pose.Orientation.y - Angle::DegToRad(2.8f));

						spider->Velocity = 48 - (abs(angle) / Angle::DegToRad(5.6f));
					}
					else
					{
						if (spider->Velocity < (i & 0x1F) + 24)
							spider->Velocity++;

						if (abs(angle) >= Angle::DegToRad(11.25f))
						{
							if (angle >= 0)
								spider->Pose.Orientation.SetY(spider->Pose.Orientation.y + Angle::DegToRad(5.6f));
							else
								spider->Pose.Orientation.SetY(spider->Pose.Orientation.y - Angle::DegToRad(5.6f));
						}
						else
							spider->Pose.Orientation.y += Angle::ShrtToRad((Wibble - i) * 8);
					}
				}

				FloorInfo* floor = GetFloor(spider->Pose.Position.x, spider->Pose.Position.y, spider->Pose.Position.z,&spider->RoomNumber);
				int height = GetFloorHeight(floor, spider->Pose.Position.x, spider->Pose.Position.y, spider->Pose.Position.z);

				if (height >= spider->Pose.Position.y - CLICK(5) || height == -SECTOR(31.75f))
				{
					if (height >= spider->Pose.Position.y - 64)
					{
						if (spider->Pose.Position.y <= height)
						{
							if (spider->VerticalVelocity >= 500)
							{
								spider->On = false;
								NextSpider = 0;
							}
							else
								spider->Pose.Orientation.x = Angle::ShrtToRad(-128 * spider->VerticalVelocity);
						}
						else
						{
							spider->Pose.Position.y = height;
							spider->VerticalVelocity = 0;
							spider->Flags = 1;
						}
					}
					else
					{
						spider->Pose.Position = oldPos;
						spider->Pose.Position.y -= 8;
						spider->Pose.Orientation.x = Angle::DegToRad(78.75f);
						spider->VerticalVelocity = 0;

						if (!(GetRandomControl() & 0x1F))
							spider->Pose.Orientation.SetY(spider->Pose.Orientation.y + Angle::DegToRad(-180.0f));
					}
				}
				else
				{
					if (angle <= 0)
						spider->Pose.Orientation.SetY(spider->Pose.Orientation.y - Angle::DegToRad(90.0f));
					else
						spider->Pose.Orientation.SetY(spider->Pose.Orientation.y + Angle::DegToRad(90.0f));

					spider->Pose.Position = oldPos;
					spider->VerticalVelocity = 0;
				}

				if (spider->Pose.Position.y < g_Level.Rooms[spider->RoomNumber].maxceiling + 50)
				{
					spider->Pose.Position.y = g_Level.Rooms[spider->RoomNumber].maxceiling + 50;
					spider->Pose.Orientation.SetY(spider->Pose.Orientation.y + Angle::DegToRad(-180.0f));
					spider->VerticalVelocity = 1;
				}

				if (!i && !(GetRandomControl() & 4))
					SoundEffect(SFX_TR5_INSECTS,&spider->Pose);
			}
		}
	}
}
