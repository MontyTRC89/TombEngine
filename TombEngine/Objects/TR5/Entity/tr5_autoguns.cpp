#include "framework.h"
#include "tr5_autoguns.h"
#include "Game/collision/sphere.h"
#include "Game/Lara/lara.h"
#include "Game/animation.h"
#include "Game/control/los.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/level.h"
#include "Sound/sound.h"
#include "Game/items.h"

void InitialiseAutoGuns(short itemNumber)
{
    auto* item = &g_Level.Items[itemNumber];

    item->MeshBits = 1024;
    //5702 bytes!?
	//item->data = game_malloc<uint8_t>(5702);
	item->Data = std::array<short,4>();
	
}

static void TriggerAutoGunSmoke(Vector3Int* pos, char shade)
{
	auto* spark = &SmokeSparks[GetFreeSmokeSpark()];

	spark->on = 1;
	spark->sShade = 0;
	spark->dShade = shade;
	spark->colFadeSpeed = 4;
	spark->fadeToBlack = 32;
	spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
	spark->life = spark->sLife = (GetRandomControl() & 3) + 40;
	spark->x = pos->x - 16 + (GetRandomControl() & 0x1F);
	spark->y = (GetRandomControl() & 0x1F) + pos->y - 16;
	spark->xVel = 0;
	spark->yVel = 0;
	spark->zVel = 0;
	spark->z = (GetRandomControl() & 0x1F) + pos->z - 16;
	spark->friction = 4;
	spark->flags = SP_ROTATE;
	spark->rotAng = GetRandomControl() & 0xFFF;
	spark->rotAdd = (GetRandomControl() & 0x3F) - 31;
	spark->maxYvel = 0;
	spark->gravity = -4 - (GetRandomControl() & 3);
	spark->mirror = 0;
	spark->dSize = (GetRandomControl() & 0xF) + 24;
	spark->sSize = spark->dSize / 4;
	spark->size = spark->dSize / 4;
}

void AutoGunsControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		if (item->Animation.FrameNumber >= g_Level.Anims[item->Animation.AnimNumber].frameEnd)
		{
			std::array<short, 4>& data = item->Data;

			item->MeshBits = 1664;

			GameVector pos1 = { 0, 0, -64 };
			GetJointAbsPosition(item, (Vector3Int*)&pos1, 8);

			GameVector pos2 = { 0, 0, 0 };
			GetLaraJointPosition((Vector3Int*)&pos2, 0);

			pos1.roomNumber = item->RoomNumber;

			int los = LOS(&pos1, &pos2);
			Vector3Shrt angles;

			// FIXME:
			if (los)
			{
				angles = GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z);
				angles.y -= item->Pose.Orientation.y;
			}
			else
			{
				angles.x = item->ItemFlags[1];
				angles.y = item->ItemFlags[0];
			}

			short angle1, angle2;
			InterpolateAngle(angles.x, &item->ItemFlags[1], &angle2, 4);
			InterpolateAngle(angles.y, item->ItemFlags, &angle1, 4);

			data[0] = item->ItemFlags[0];
			data[1] = item->ItemFlags[1];
			data[2] += item->ItemFlags[2];

			if (abs(angle1) < 1024 && abs(angle2) < 1024 && los)
			{
				SoundEffect(SFX_TR4_HK_FIRE, &item->Pose, SoundEnvironment::Land, 0.8f);

				if (GlobalCounter & 1)
				{
					item->MeshBits |= 0x100;

					TriggerDynamicLight(pos1.x, pos1.y, pos1.z, 10, (GetRandomControl() & 0x1F) + 192, (GetRandomControl() & 0x1F) + 128, 0);

					if (GetRandomControl() & 3)
					{
						auto pos2 = Vector3Int();
						GetLaraJointPosition((Vector3Int*)& pos2, GetRandomControl() % 15);

						DoBloodSplat(pos2.x, pos2.y, pos2.z, (GetRandomControl() & 3) + 3, 2 * GetRandomControl(), LaraItem->RoomNumber);
						DoDamage(LaraItem, 20);
					}
					else
					{
						GameVector pos;
						pos.x = pos2.x;
						pos.y = pos2.y;
						pos.z = pos2.z;

						int dx = pos2.x - pos1.x;
						int dy = pos2.y - pos1.y;
						int dz = pos2.z - pos1.z;

						while (true)
						{
							if (abs(dx) >= 12288)
								break;
							if (abs(dy) >= 12288)
								break;
							if (abs(dz) >= 12288)
								break;
							dx *= 2;
							dy *= 2;
							dz *= 2;
						}

						pos.x += dx + GetRandomControl() - 128;
						pos.y += dy + GetRandomControl() - 128;
						pos.z += dz + GetRandomControl() - 128;

						if (!LOS(&pos1, &pos))
							TriggerRicochetSpark(&pos, 2 * GetRandomControl(), 3, 0);
					}
				}
				else
					item->MeshBits &= ~0x100;

				if (item->ItemFlags[2] < 1024)
					item->ItemFlags[2] += 64;
			}
			else
			{
				if (item->ItemFlags[2])
					item->ItemFlags[2] -= 64;
				item->MeshBits &= ~0x100;
			}

			if (item->ItemFlags[2])
				TriggerAutoGunSmoke((Vector3Int*)&pos1, item->ItemFlags[2] / 16);
		}
		else
		{
			item->MeshBits = 0xFFFFFAFF;
			AnimateItem(item);
		}
	}
}
