#include "framework.h"
#include "tr5_laser_head.h"
#include "Game/items.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/effects.h"
#include "Game/people.h"
#include "Game/effects/debris.h"
#include "Game/animation.h"
#include "Game/control/los.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Objects/TR5/Entity/tr5_laserhead_info.h"
#include "Game/effects/lightning.h"
#include "Game/effects/lara_fx.h"
#include "Game/misc.h"

using namespace TEN::Effects::Lara;
using namespace TEN::Effects::Lightning;

struct LaserHeadStruct
{
	Vector3Int target;
	LIGHTNING_INFO* fireArcs[2];
	LIGHTNING_INFO* chargeArcs[4];
	bool LOS[2];
	byte byte1;
	byte byte2;
	short xRot;
	short yRot;
};

LaserHeadStruct LaserHeadData;

auto LaserHeadBasePosition = Vector3Int(0, -640, 0);
Vector3Int GuardianChargePositions[4] = { Vector3Int(-188, -832, 440), Vector3Int(188, -832, -440), Vector3Int(440, -832, 188), Vector3Int(-440, -832, -188) };
int GuardianMeshes[5] = { 1, 2 };

static void TriggerLaserHeadSparks(Vector3Int* pos, int count, byte r, byte g, byte b, int unk)
{
	if (count > 0)
	{
		for (int i = 0; i < count; i++)
		{
			auto* spark = GetFreeParticle();

			spark->on = 1;
			spark->sR = r;
			spark->sG = g;
			spark->sB = b;
			spark->dB = 0;
			spark->dG = 0;
			spark->dR = 0;
			spark->colFadeSpeed = 9 << unk;
			spark->fadeToBlack = 0;
			spark->life = 9 << unk;
			spark->sLife = 9 << unk;
			spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
			spark->x = pos->x;
			spark->y = pos->y;
			spark->z = pos->z;
			spark->gravity = (GetRandomControl() / 128) & 0x1F;
			spark->yVel = ((GetRandomControl() & 0xFFF) - 2048) << unk;
			spark->xVel = ((GetRandomControl() & 0xFFF) - 2048) << unk;
			spark->zVel = ((GetRandomControl() & 0xFFF) - 2048) << unk;
			spark->flags = SP_NONE;
			spark->maxYvel = 0;
			spark->friction = 34 << unk;
		}
	}
}

static void LaserHeadCharge(ItemInfo* item)
{
	byte size = item->ItemFlags[3];
	byte g = ((GetRandomControl() & 0x1F) + 128);
	byte b = ((GetRandomControl() & 0x1F) + 64);

	if (item->ItemFlags[3] <= 32)
	{
		g = (item->ItemFlags[3] * g) / 32;
		b = (item->ItemFlags[3] * b) / 32;
	}
	else
		size = 32;

	auto* creature = (LaserHeadInfo*)item->Data;

	auto dest = Vector3Int(LaserHeadBasePosition.x, LaserHeadBasePosition.y, LaserHeadBasePosition.z);
	GetJointAbsPosition(&g_Level.Items[creature->BaseItem], &dest, 0);

	Vector3Int src;

	for (int i = 0; i < 4; i++)
	{
		auto* arc = LaserHeadData.chargeArcs[i];

		if (item->ItemFlags[3] & 0x0F && arc != NULL)
		{
			arc->r = b;
			arc->g = g;
			arc->b = 0;
			arc->life = 50;
		}
		else
		{
			src = Vector3Int(GuardianChargePositions[i].x, GuardianChargePositions[i].y, GuardianChargePositions[i].z);
			GetJointAbsPosition(&g_Level.Items[creature->BaseItem], &src, 0);
			//LaserHeadData.chargeArcs[i] = TriggerEnergyArc(&src, &dest, 0, g, b, 256, 90, 64, ENERGY_ARC_NO_RANDOMIZE, ENERGY_ARC_STRAIGHT_LINE); //  (GetRandomControl() & 7) + 8, v4 | ((v1 | 0x240000) << 8), 13, 48, 3);
		}
	}

	if (GlobalCounter & 1)
	{
		for (int i = 0; i < 2; i++)
		{
			if (1 << GuardianMeshes[i] & item->MeshBits)
			{
				src = Vector3Int();
				GetJointAbsPosition(item, &src, GuardianMeshes[i]);

				TriggerLightningGlow(src.x, src.y, src.z, size + (GetRandomControl() & 3), 0, g, b);
				TriggerLaserHeadSparks(&src, 3, 0, g, b, 0);
			}
		}

		TriggerLightningGlow(dest.x, dest.y, dest.z, (GetRandomControl() & 3) + size + 8, 0, g, b);
		TriggerDynamicLight(dest.x, dest.y, dest.z, (GetRandomControl() & 3) + 16, 0, g, b);
	}

	if (!(GlobalCounter & 3))
	{
		//TriggerEnergyArc(&dest, (Vector3Int*)&item->pos, 0, g, b, 256, 3, 64, ENERGY_ARC_NO_RANDOMIZE, ENERGY_ARC_STRAIGHT_LINE);
		//TriggerEnergyArc(&dest, &item->pos, (GetRandomControl() & 7) + 8, v4 | ((v1 | 0x180000) << 8), 13, 64, 3);
	}

	TriggerLaserHeadSparks(&dest, 3, 0, g, b, 1);
}

void InitialiseLaserHead(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	item->Data = LaserHeadInfo();
	auto info = (LaserHeadInfo*)item->Data;

	for (int i = 0; i < g_Level.NumItems; i++)
	{
		if (g_Level.Items[i].ObjectNumber == ID_LASERHEAD_BASE)
		{
			info->BaseItem = i;
			break;
		}
	}

	short rotation = 0;
	for (int j = 0; j < 8; j++)
	{
		for (int i = 0; i < g_Level.NumItems; i++)
		{
			if (g_Level.Items[i].ObjectNumber == ID_LASERHEAD_TENTACLE && g_Level.Items[i].Pose.Orientation.y == rotation)
			{
				info->Tentacles[j] = i;
				break;
			}
		}

		rotation += ANGLE(45.0f);
	}

	for (int i = 0; i < g_Level.NumItems; i++)
	{
		if (g_Level.Items[i].ObjectNumber == ID_PUZZLE_ITEM4)
		{
			info->PuzzleItem = i;
			break;
		}
	}

	int y = item->Pose.Position.y - 640;

	item->Pose.Position.y = y;
	item->Animation.ActiveState = 0;
	item->ItemFlags[1] = y - 640;
	item->ItemFlags[3] = 90;

	ZeroMemory(&LaserHeadData, sizeof(LaserHeadStruct));
}

void LaserHeadControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	auto* creature = (LaserHeadInfo*)item->Data;

	GameVector src, dest;

	// NOTICE: itemFlags[0] seems to be a state machine, if it's equal to 3 then death animations is triggered
	// Other values still unknown
	
	if (item->ItemFlags[0])
	{
		// Maybe number of eye hits?
		if (item->ItemFlags[0] > 2)
		{
			if (!(GlobalCounter & 7))
			{
				if (item->Animation.ActiveState < 8)
				{
					short tentacleNumber = creature->Tentacles[item->Animation.ActiveState];
					g_Level.Items[tentacleNumber].Animation.TargetState = 2;
					item->Animation.ActiveState++;
				}
			}

			// Destroy tentacle items
			if (item->Animation.ActiveState > 0)
			{
				for (int i = 0; i < 8; i++)
				{
					auto* tentacleItem = &g_Level.Items[creature->Tentacles[i]];

					if (tentacleItem->Animation.AnimNumber == Objects[tentacleItem->ObjectNumber].animIndex + 1 &&
						tentacleItem->Animation.FrameNumber == g_Level.Anims[tentacleItem->Animation.AnimNumber].frameEnd &&
						tentacleItem->MeshBits & 1)
					{
						SoundEffect(SFX_TR5_SMASH_ROCK2, &item->Pose);
						ExplodeItemNode(tentacleItem, 0, 0, 128);
						KillItem(creature->Tentacles[i]);
					}
				}
			}

			item->Pose.Position.y = item->ItemFlags[1] - (192 - item->Animation.Velocity) * phd_sin(item->ItemFlags[2]);
			item->ItemFlags[2] += ONE_DEGREE * item->Animation.Velocity;

			if (!(GlobalCounter & 7))
			{
				item->ItemFlags[3] = item->Pose.Orientation.y + (GetRandomControl() & 0x3FFF) - 4096;
				item->TriggerFlags = (GetRandomControl() & 0x1000) - 2048;
			}

			InterpolateAngle(item->ItemFlags[3], &item->Pose.Orientation.y, 0, 2);
			InterpolateAngle(item->TriggerFlags, &item->Pose.Orientation.x, 0, 2);

			// Final death
			item->Animation.Velocity++;
			if (item->Animation.Velocity > 136)
			{
				ExplodeItemNode(&g_Level.Items[creature->BaseItem], 0, 0, 128);
				KillItem(creature->BaseItem);

				ExplodeItemNode(item, 0, 0, 128);

				item->Pose.Position.y -= 256;
				TriggerExplosionSparks(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, 3, -2, 2, item->RoomNumber);
				TriggerExplosionSparks(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, 2, 0, 2, item->RoomNumber);
				
				TriggerShockwave(&item->Pose, 32, 160, 64, 64, 128, 0, 36, 0, 0);
				TriggerShockwave(&item->Pose, 32, 160, 64, 64, 128, 0, 36, 0x3000, 0);
				TriggerShockwave(&item->Pose, 32, 160, 64, 64, 128, 0, 36, 0x6000, 0);

				g_Level.Items[creature->PuzzleItem].Pose.Position.y = item->Pose.Position.y;
				TestTriggers(item, true);

				SoundEffect(SFX_TR5_GOD_HEAD_BLAST, &item->Pose, SoundEnvironment::Land, 0.5f);
				SoundEffect(SFX_TR4_EXPLOSION2, &item->Pose, SoundEnvironment::Land, 1.25f);
				SoundEffect(SFX_TR4_EXPLOSION1, &item->Pose);
				SoundEffect(SFX_TR4_EXPLOSION1, &item->Pose, SoundEnvironment::Land, 0.25f);

				KillItem(itemNumber);
			}
		}
		else
		{
			item->TriggerFlags++;
			item->Pose.Position.y = item->ItemFlags[1] - 128 * phd_sin(item->ItemFlags[2]);
			item->ItemFlags[2] += ANGLE(3.0f);

			// Get guardian head's position
			src.x = 0;
			src.y = 168;
			src.z = 248;
			src.roomNumber = item->RoomNumber;
			GetJointAbsPosition(item, (Vector3Int*)&src, 0);

			if (item->ItemFlags[0] == 1)
			{
				// Get Lara's left hand position
				// TODO: check if left hand or head
				dest.x = 0;
				dest.y = 0;
				dest.z = 0;
				GetJointAbsPosition(LaraItem, (Vector3Int*)&dest, LM_HEAD);

				// Calculate distance between guardian and Lara
				int distance = sqrt(pow(src.x - dest.x, 2) + pow(src.y - dest.y, 2) + pow(src.z - dest.z, 2));

				// Check if there's a valid LOS between guardian and Lara 
				// and if distance is less than 8 sectors  and if Lara is alive and not burning
				if (LOS(&src, &dest) &&
					distance <= MAX_VISIBILITY_DISTANCE &&
					LaraItem->HitPoints > 0 &&
					!Lara.Burn &&
					(LaserHeadData.target.x || LaserHeadData.target.y || LaserHeadData.target.z))
				{
					// Lock target for attacking
					dest.x = 0;
					dest.y = 0;
					dest.z = 0;
					GetJointAbsPosition(LaraItem, (Vector3Int*)&dest, LM_HIPS);

					LaserHeadData.target.x = dest.x;
					LaserHeadData.target.y = dest.y;
					LaserHeadData.target.z = dest.z;
					LaserHeadData.byte1 = 3;
					LaserHeadData.byte2 = 1;
				}
				else
				{
					// Randomly turn head try to finding Lara
					bool condition = !(GetRandomControl() & 0x7F) && item->TriggerFlags > 150;

					if (item->ItemFlags[3]-- <= 0 || condition)
					{
						short xRot = (GetRandomControl() / 4) - 4096;
						short yRot;
						if (condition)
							yRot = item->Pose.Orientation.y + (GetRandomControl() & 0x3FFF) + ANGLE(135.0f);
						else
							yRot = 2 * GetRandomControl();
						int v = ((GetRandomControl() & 0x1FFF) + 8192);
						int c = v * phd_cos(-xRot);
						dest.x = src.x + c * phd_sin(yRot);
						dest.y = src.y + v * phd_sin(-xRot);
						dest.z = src.z + c * phd_cos(yRot);

						if (condition)
						{
							LaserHeadData.byte1 = 2;
							item->TriggerFlags = 0;
						}
						else
							LaserHeadData.byte1 = (GetRandomControl() & 2) + 3;

						item->ItemFlags[3] = LaserHeadData.byte1 * ((GetRandomControl() & 3) + 8);

						LaserHeadData.target.x = dest.x;
						LaserHeadData.target.y = dest.y;
						LaserHeadData.target.z = dest.z;
					}
					else
					{
						dest.x = LaserHeadData.target.x;
						dest.y = LaserHeadData.target.y;
						dest.z = LaserHeadData.target.z;
					}

					LaserHeadData.byte2 = 0;
				}
			}
			else
			{
				LaserHeadData.byte1 = 3;

				if (JustLoaded)
				{
					int c = 8192 * phd_cos(item->Pose.Orientation.x + 3328);
					
					dest.x = LaserHeadData.target.x = src.x + c * phd_sin(item->Pose.Orientation.y);
					dest.y = LaserHeadData.target.y = src.y + SECTOR(8) * phd_sin(3328 - item->Pose.Orientation.x);
					dest.z = LaserHeadData.target.z = src.z + c * phd_cos(item->Pose.Orientation.y);
				}
				else
				{
					dest.x = LaserHeadData.target.x;
					dest.y = LaserHeadData.target.y;
					dest.z = LaserHeadData.target.z;
				}
			}

			auto angles = GetVectorAngles(LaserHeadData.target.x - src.x, LaserHeadData.target.y - src.y, LaserHeadData.target.z - src.z);
			InterpolateAngle(angles.x + 3328, &item->Pose.Orientation.x, &LaserHeadData.xRot, LaserHeadData.byte1);
			InterpolateAngle(angles.y, &item->Pose.Orientation.y, &LaserHeadData.yRot, LaserHeadData.byte1);

			if (item->ItemFlags[0] == 1)
			{
				if (LaserHeadData.byte2)
				{
					if (!(GetRandomControl() & 0x1F) &&
						abs(LaserHeadData.xRot) < ANGLE(5.6f) &&
						abs(LaserHeadData.yRot) < ANGLE(5.6f) &&
						!LaraItem->Animation.VerticalVelocity ||
						!(GetRandomControl() & 0x1FF))
					{
						item->ItemFlags[0]++;
						item->ItemFlags[3] = 0;
					}
				}
				else if (!(GetRandomControl() & 0x3F) && item->TriggerFlags > 300)
				{
					item->ItemFlags[0]++;
					item->TriggerFlags = 0;
					item->ItemFlags[3] = 0;
				}
			}
			else
			{
				if (item->ItemFlags[3] <= 90)
				{
					SoundEffect(SFX_TR5_GOD_HEAD_CHARGE, &item->Pose);
					LaserHeadCharge(item);
					item->ItemFlags[3]++;
				}

				if (item->ItemFlags[3] >= 90)
				{
					byte r = 0;
					byte g = (GetRandomControl() & 0x1F) + 128;
					byte b = (GetRandomControl() & 0x1F) + 64;

					auto* arc = LaserHeadData.fireArcs[0];
					if (!LaserHeadData.fireArcs[0])
						arc = LaserHeadData.fireArcs[1];

					if (item->ItemFlags[3] > 90 &&
						arc &&
						!arc->life ||
						LaraItem->HitPoints <= 0 ||
						Lara.Burn)
					{
						if (arc)
						{
							LaserHeadData.fireArcs[0] = NULL;
							LaserHeadData.fireArcs[1] = NULL;
						}

						item->ItemFlags[0] = 1;
						item->TriggerFlags = 0;
					}
					else
					{
						if (item->ItemFlags[3] > 90 &&
							arc &&
							arc->life < 16)
						{
							g = (arc->life * g) / 16;
							b = (arc->life * b) / 16;
						}

						for (int i = 0; i < 2; i++)
						{ 
							// If eye was not destroyed then fire from it
							if ((1 << GuardianMeshes[i]) & item->MeshBits)
							{
								src.x = 0;
								src.y = 0;
								src.z = 0;
								GetJointAbsPosition(item, (Vector3Int*)& src, GuardianMeshes[i]);

								int c = 8192 * phd_cos(angles.x);
								dest.x = src.x + c * phd_sin(item->Pose.Orientation.y);
								dest.y = src.y + 8192 * phd_sin(-angles.x);
								dest.z = src.z + c * phd_cos(item->Pose.Orientation.y);

								if (item->ItemFlags[3] != 90 &&
									LaserHeadData.fireArcs[i] != NULL)
								{
									// Eye is aready firing
									SoundEffect(SFX_TR5_GOD_HEAD_LASER_LOOPS, &item->Pose);

									LaserHeadData.fireArcs[i]->pos1.x = src.x;
									LaserHeadData.fireArcs[i]->pos1.y = src.y;
									LaserHeadData.fireArcs[i]->pos1.z = src.z;
								}
								else
								{
									// Start firing from eye
									src.roomNumber = item->RoomNumber;
									LaserHeadData.LOS[i] = LOS(&src, &dest);
									//LaserHeadData.fireArcs[i] = TriggerEnergyArc((Vector3Int*)& src, (Vector3Int*)& dest, r, g, b, 32, 64, 64, ENERGY_ARC_NO_RANDOMIZE, ENERGY_ARC_STRAIGHT_LINE); // (GetRandomControl() & 7) + 4, b | ((&unk_640000 | g) << 8), 12, 64, 5);
									StopSoundEffect(SFX_TR5_GOD_HEAD_CHARGE);
									SoundEffect(SFX_TR5_GOD_HEAD_BLAST, &item->Pose);
								}

								auto* currentArc = LaserHeadData.fireArcs[i];

								if (GlobalCounter & 1) 
								{
									TriggerLaserHeadSparks((Vector3Int*)&src, 3, r, g, b, 0);
									TriggerLightningGlow(src.x, src.y, src.z, (GetRandomControl() & 3) + 32, r, g, b);
									TriggerDynamicLight(src.x, src.y, src.z, (GetRandomControl() & 3) + 16, r, g, b);

									if (!LaserHeadData.LOS[i])
									{
										TriggerLightningGlow(currentArc->pos4.x, currentArc->pos4.y, currentArc->pos4.z, (GetRandomControl() & 3) + 16, r, g, b);
										TriggerDynamicLight(currentArc->pos4.x, currentArc->pos4.y, currentArc->pos4.z, (GetRandomControl() & 3) + 6, r, g, b);
										TriggerLaserHeadSparks((Vector3Int*)& currentArc->pos4, 3, r, g, b, 0);
									}
								}

								// Check if Lara was hit by energy arcs
								if (!Lara.Burn)
								{
									int someIndex = 0;

									auto* bounds = GetBoundsAccurate(LaraItem);
									BOUNDING_BOX tbounds;
									phd_RotBoundingBoxNoPersp(&LaraItem->Pose, bounds, &tbounds);

									int x1 = LaraItem->Pose.Position.x + tbounds.X1;
									int x2 = LaraItem->Pose.Position.x + tbounds.X2;
									int y1 = LaraItem->Pose.Position.y + tbounds.Y1;
									int y2 = LaraItem->Pose.Position.y + tbounds.Y1;
									int z1 = LaraItem->Pose.Position.z + tbounds.Z1;
									int z2 = LaraItem->Pose.Position.z + tbounds.Z2;

									int xc = LaraItem->Pose.Position.x + ((bounds->X1 + bounds->X2) / 2);
									int yc = LaraItem->Pose.Position.y + ((bounds->Y1 + bounds->Y2) / 2);
									int zc = LaraItem->Pose.Position.z + ((bounds->Z1 + bounds->Z2) / 2);

									int distance = sqrt(pow(xc - src.x, 2) + pow(yc - src.y, 2) + pow(zc - src.z, 2));

									if (distance < MAX_VISIBILITY_DISTANCE)
									{
										int dl = distance + CLICK(2);

										if (dl < MAX_VISIBILITY_DISTANCE)
										{
											dest.x = src.x + dl * (dest.x - src.x) / MAX_VISIBILITY_DISTANCE;
											dest.y = src.y + dl * (dest.y - src.y) / MAX_VISIBILITY_DISTANCE;
											dest.z = src.z + dl * (dest.z - src.z) / MAX_VISIBILITY_DISTANCE;
										}

										int dx = (dest.x - src.x) / 32;
										int dy = (dest.y - src.y) / 32;
										int dz = (dest.z - src.z) / 32;

										int adx = currentArc->pos4.x - src.z;
										int ady = currentArc->pos4.y - src.y;
										int adz = currentArc->pos4.z - src.z;

										int x = src.x;
										int y = src.y;
										int z = src.z;

										for (int j = 0; j < 32; j++)
										{
											if (someIndex)
											{
												someIndex--;
												if (!someIndex)
													break;
											}

											if (abs(adx) < 280 && abs(ady) < 280 && abs(adz) < 280)
												someIndex = 2;

											if (x > x1 && x < x2 && y > y1 && y < y2 && z > z1 && z < z2)
											{
												LaraBurn(LaraItem);
												DoDamage(LaraItem, INT_MAX);
												Lara.BurnCount = 48;
												Lara.BurnBlue = 2;
												LaraItem->HitPoints = 0;
												break;
											}

											x += dx;
											y += dy;
											z += dz;

											adx -= dx;
											ady -= dy;
											adz -= dz;
										}
									}
								}
							}
							else if (item->ItemFlags[3] > 90)
							{
								if (LaserHeadData.fireArcs[i])
								{
									LaserHeadData.fireArcs[i]->life = 0;
									LaserHeadData.fireArcs[i] = NULL;
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		if (item->ItemFlags[2] >= 8)
		{
			if (item->Pose.Position.y <= item->ItemFlags[1])
			{
				src.x = 0;
				src.y = 168;
				src.z = 248;
				src.roomNumber = item->RoomNumber;
				GetJointAbsPosition(item, (Vector3Int*)& src, 0);

				dest.x = 0;
				dest.y = 0;
				dest.z = 0;
				GetJointAbsPosition(LaraItem, (Vector3Int*)& dest, LM_HEAD);

				if (LOS(&src, &src))
				{
					item->ItemFlags[0]++;
					item->ItemFlags[1] = item->Pose.Position.y;
					item->ItemFlags[2] = 2640;
				}
			}
			else
			{
				item->Animation.VerticalVelocity += 3;

				if (item->Animation.VerticalVelocity > 32)
					item->Animation.VerticalVelocity = 32;

				item->Pose.Position.y -= item->Animation.VerticalVelocity;
			}
		}
		else if (!(GlobalCounter & 7))
		{
			short tentacleItemNumber = creature->Tentacles[item->ItemFlags[2]];
			auto* tentacleItem = &g_Level.Items[tentacleItemNumber];
			AddActiveItem(tentacleItemNumber);
			tentacleItem->Status = ITEM_ACTIVE;
			tentacleItem->Flags |= 0x3E00;
			item->ItemFlags[2]++;
		}
	}

	if (item->ItemFlags[0] < 3)
	{
		int i = 0;
		for (i = 0; i < 8; i++)
		{
			short tentacleItemNumber = creature->Tentacles[i];
			auto* tentacleItem = &g_Level.Items[tentacleItemNumber];

			if (tentacleItem->Animation.AnimNumber == Objects[tentacleItem->ObjectNumber].animIndex &&
				tentacleItem->Animation.FrameNumber != g_Level.Anims[tentacleItem->Animation.AnimNumber].frameEnd)
			{
				break;
			}
		}

		// If all tentacles animations are done and both eyes are destroyed it's time to die
		if (i == 8 && !(item->MeshBits & 6))
		{
			if (LaserHeadData.fireArcs[0])
				LaserHeadData.fireArcs[0]->life = 2;
			if (LaserHeadData.fireArcs[1])
				LaserHeadData.fireArcs[1]->life = 2;

			LaserHeadData.fireArcs[0] = NULL;
			LaserHeadData.fireArcs[1] = NULL;

			item->Animation.Velocity = 3;
			item->ItemFlags[0] = 3;
			item->ItemFlags[3] = item->Pose.Orientation.y + (GetRandomControl() & 0x1000) - 2048;
			item->TriggerFlags = item->Pose.Orientation.x + (GetRandomControl() & 0x1000) - 2048;
		}
	}
}
