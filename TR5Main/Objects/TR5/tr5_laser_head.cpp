#include "../oldobjects.h"
#include "../../Game/sphere.h"
#include "../../Game/items.h"
#include "../../Game/tomb4fx.h"
#include "../../Game/effect2.h"
#include "../../Game/Box.h"
#include "../../Game/people.h"
#include "../../Game/debris.h"
#include "../../Game/draw.h"
#include "../../Game/control.h"
#include "../../Game/effects.h"
#include "../../Game/switch.h"
#include "../../Game/laramisc.h"
#include "../../Game/traps.h"
#include "../../Specific/setup.h"

struct LASER_HEAD_INFO
{
	short baseItem;
	short tentacles[8];
	short puzzleItem;
};

struct LASER_HEAD_STRUCT
{
	PHD_VECTOR target;
	ENERGY_ARC* fireArcs[2];
	ENERGY_ARC* chargeArcs[8];
	bool LOS;
	byte byte1;
	byte byte2;
	short xRot;
	short yRot;
};

LASER_HEAD_STRUCT LaserHeadData;

PHD_VECTOR LaserHeadBasePosition = { 0, -640, 0 };
PHD_VECTOR GuardianChargePositions[8];
int GuardianMeshes[5] = { 1,0,0,0,2 };

void InitialiseGuardian(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	item->data = (LASER_HEAD_INFO*)GameMalloc(sizeof(LASER_HEAD_INFO));
	LASER_HEAD_INFO* info = (LASER_HEAD_INFO*)item->data;

	for (int i = 0; i < LevelItems; i++)
	{
		if (Items[i].objectNumber == ID_GUARDIAN_BASE)
		{
			info->baseItem = i;
			break;
		}
	}

	short rotation = 0;
	int j = 0;
	for (int i = 0; i < LevelItems; i++)
	{
		if (Items[i].objectNumber == ID_GUARDIAN_TENTACLE && j < 8)
		{
			info->tentacles[j] = i;
			rotation += ANGLE(45);
			j++;
		}
	}

	for (int i = 0; i < LevelItems; i++)
	{
		if (Items[i].objectNumber == ID_PUZZLE_ITEM4)
		{
			info->puzzleItem = i;
			break;
		}
	}

	int y = item->pos.yPos - 640;
	item->pos.yPos = y;
	item->itemFlags[1] = y - 640;
	item->currentAnimState = 0;
	item->itemFlags[3] = 90;

	ZeroMemory(&LaserHeadData, sizeof(LASER_HEAD_STRUCT));
}

void GuardianControl(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	LASER_HEAD_INFO* creature = (LASER_HEAD_INFO*)item->data;

	GAME_VECTOR src, dest;
	printf("Guardian Y: %d\n", item->pos.yPos);
	if (item->itemFlags[0])
	{
		// Maybe number of eye hits?
		if (item->itemFlags[0] > 2)
		{
			if (!(GlobalCounter & 7))
			{
				if (item->currentAnimState < 8)
				{
					short tentacleNumber = creature->tentacles[item->currentAnimState];
					Items[tentacleNumber].goalAnimState = 2;
					item->currentAnimState++;
				}
			}

			// Destroy tentacle items
			if (item->currentAnimState > 0)
			{
				for (int i = 0; i < 8; i++)
				{
					ITEM_INFO* tentacleItem = &Items[creature->tentacles[i]];

					if (tentacleItem->animNumber == Objects[tentacleItem->objectNumber].animIndex + 1
						&& tentacleItem->frameNumber == Anims[tentacleItem->animNumber].frameEnd
						&& tentacleItem->meshBits & 1)
					{
						SoundEffect(SFX_SMASH_ROCK, &item->pos, 0);
						ExplodeItemNode(tentacleItem, 0, 0, 128);
						KillItem(creature->tentacles[i]);
					}
				}
			}

			item->pos.yPos = item->itemFlags[1] - ((192 - item->speed) * SIN(item->itemFlags[2]) >> W2V_SHIFT);
			item->itemFlags[2] += ONE_DEGREE * item->speed;

			if (!(GlobalCounter & 7))
			{
				item->itemFlags[3] = item->pos.yRot + (GetRandomControl() & 0x3FFF) - 4096;
				item->triggerFlags = (GetRandomControl() & 0x1000) - 2048;
			}

			InterpolateAngle(item->itemFlags[3], &item->pos.yRot, 0, 2);
			InterpolateAngle(item->triggerFlags, &item->pos.xRot, 0, 2);

			// Final death
			item->speed++;
			if (item->speed > 136)
			{
				ExplodeItemNode(&Items[creature->baseItem], 0, 0, 128);
				KillItem(creature->baseItem);

				ExplodeItemNode(item, 0, 0, 128);

				TriggerExplosionSparks(item->pos.xPos, item->pos.yPos - 256, item->pos.zPos, 3, -2, 2, item->roomNumber);
				TriggerExplosionSparks(item->pos.xPos, item->pos.yPos, item->pos.zPos, 2, 0, 2, item->roomNumber);
				
				TriggerShockwave(&item->pos, 32, 160, 64, 64, 128, 0, 36, 0, 0);
				TriggerShockwave(&item->pos, 32, 160, 64, 64, 128, 0, 36, 0, 3);
				TriggerShockwave(&item->pos, 32, 160, 64, 64, 128, 0, 36, 0, 6);

				Items[creature->puzzleItem].pos.yPos = item->pos.yPos;
				TestTriggersAtXYZ(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 1, 0);

				SoundEffect(SFX_GOD_HEAD_BLAST, &item->pos, 0x800004);
				SoundEffect(SFX_EXPLOSION2, &item->pos, 20971524);
				SoundEffect(SFX_EXPLOSION1, &item->pos, 0);
				SoundEffect(SFX_EXPLOSION1, &item->pos, 4194308);

				KillItem(itemNumber);
			}
		}
		else
		{
			item->triggerFlags++;
			item->pos.yPos = item->itemFlags[1] - (128 * SIN(item->itemFlags[2]) >> W2V_SHIFT);
			item->itemFlags[2] += ANGLE(3);

			// Get guardian head's position
			src.x = 0;
			src.y = 168;
			src.z = 248;
			src.roomNumber = item->roomNumber;
			GetJointAbsPosition(item, (PHD_VECTOR*)&src, 0);

			if (item->itemFlags[0] == 1)
			{
				// Get Lara's left hand position
				// TODO: check if left hand or head
				dest.x = 0;
				dest.y = 0;
				dest.z = 0;
				GetJointAbsPosition(LaraItem, (PHD_VECTOR*)&dest, LJ_LHAND);

				// Calculate distance between guardian and Lara
				int distance = SQRT_ASM(SQUARE(src.x - dest.x) + SQUARE(src.y - dest.y) + SQUARE(src.z - dest.z));

				// Check if there's a valid LOS between guardian and Lara 
				// and if distance is less than 8 sectors  and if Lara is alive and not burning
				if (LOS(&src, &dest)
					&& distance <= 8192
					&& LaraItem->hitPoints > 0
					&& !Lara.burn
					&& (LaserHeadData.target.x || LaserHeadData.target.y || LaserHeadData.target.z))
				{
					// Lock target for attacking
					dest.x = 0;
					dest.y = 0;
					dest.z = 0;
					GetJointAbsPosition(LaraItem, (PHD_VECTOR*)&dest, LJ_HIPS);

					LaserHeadData.target.x = dest.x;
					LaserHeadData.target.y = dest.y;
					LaserHeadData.target.z = dest.z;
					LaserHeadData.byte1 = 3;
					LaserHeadData.byte2 = 1;
				}
				else
				{
					// Randomly turn head try to finding Lara
					bool condition = !(GetRandomControl() & 0x7F) && item->triggerFlags > 150;
					item->itemFlags[3]--;

					if (item->itemFlags[3] <= 0 || condition)
					{
						short xRot = (GetRandomControl() >> 2) - 4096;
						short yRot;
						if (condition)
							yRot = item->pos.yRot + (GetRandomControl() & 0x3FFF) + ANGLE(135);
						else
							yRot = 2 * GetRandomControl();
						int v = ((GetRandomControl() & 0x1FFF) + 8192);
						int c = v * COS(-xRot) >> W2V_SHIFT;
						dest.x = src.x + (c * SIN(yRot) >> W2V_SHIFT);
						dest.y = src.y + (v * SIN(-xRot) >> W2V_SHIFT);
						dest.z = src.z + (c * COS(yRot) >> W2V_SHIFT);

						if (condition)
						{
							LaserHeadData.byte1 = 2;
							item->triggerFlags = 0;
						}
						else
						{
							LaserHeadData.byte1 = (GetRandomControl() & 2) + 3;
						}

						item->itemFlags[3] = LaserHeadData.byte1 * ((GetRandomControl() & 3) + 8);

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
					int c = 8192 * COS(item->pos.xRot + 3328) >> W2V_SHIFT;
					
					dest.x = LaserHeadData.target.x = src.x + (c * SIN(item->pos.yRot) >> W2V_SHIFT);
					dest.y = LaserHeadData.target.y = src.y + (8192 * SIN(3328 - item->pos.xRot) >> W2V_SHIFT);
					dest.z = LaserHeadData.target.z = src.z + (c * COS(item->pos.yRot) >> W2V_SHIFT);
				}
				else
				{
					dest.x = LaserHeadData.target.x;
					dest.y = LaserHeadData.target.y;
					dest.z = LaserHeadData.target.z;
				}
			}

			short angles[2];
			short outAngle;
			phd_GetVectorAngles(LaserHeadData.target.x - src.x, LaserHeadData.target.y - src.y, LaserHeadData.target.z - src.z, angles);
			InterpolateAngle(angles[0], &item->pos.yRot, &LaserHeadData.yRot, LaserHeadData.byte1);
			InterpolateAngle(angles[1] + 3328, &item->pos.xRot, &LaserHeadData.xRot, LaserHeadData.byte1);

			if (item->itemFlags[0] == 1)
			{
				if (LaserHeadData.byte2)
				{
					if (!(GetRandomControl() & 0x1F)
						&& abs(LaserHeadData.xRot) < 1024
						&& abs(LaserHeadData.yRot) < 1024
						&& !LaraItem->fallspeed
						|| !(GetRandomControl() & 0x1FF))
					{
						item->itemFlags[0]++;
						item->itemFlags[3] = 0;
					}
				}
				else if (!(GetRandomControl() & 0x3F) && item->triggerFlags > 300)
				{
					item->itemFlags[0]++;
					item->triggerFlags = 0;
					item->itemFlags[3] = 0;
				}
			}
			else
			{
				bool condition = false;
				if (item->itemFlags[3] <= 90)
				{
					SoundEffect(SFX_GOD_HEAD_CHARGE, &item->pos, 0);
					GuardianCharge(item);
					item->itemFlags[3]++;
					condition = item->itemFlags[3] >= 90;
				}

				if (item->itemFlags[3] > 90 || condition)
				{
					byte r, g, b;

					g = (GetRandomControl() & 0x1F) + 128;
					b = (GetRandomControl() & 0x1F) + 64;

					ENERGY_ARC* arc = LaserHeadData.fireArcs[0];
					if (!LaserHeadData.fireArcs[0])
						arc = LaserHeadData.fireArcs[1];

					if (item->itemFlags[3] > 90
						&& arc
						&& !arc->life
						|| LaraItem->hitPoints <= 0
						|| Lara.burn)
					{
						if (arc)
						{
							LaserHeadData.fireArcs[0] = NULL;
							LaserHeadData.fireArcs[1] = NULL;
						}
						item->itemFlags[0] = 1;
						item->triggerFlags = 0;
					}
					else
					{
						if (item->itemFlags[3] > 90
							&& arc
							&& arc->life < 16)
						{
							g = b = arc->life * g >> 4;
						}

						for (int i = 0, j = 0; i < 5; i += 4, j++)
						{ 
							// If eye was not destroyed then fire from it
							if ((1 << GuardianMeshes[i]) & item->meshBits)
							{
								src.x = 0;
								src.y = 0;
								src.z = 0;
								GetJointAbsPosition(item, (PHD_VECTOR*)& src, GuardianMeshes[i]);

								int c = 8192 * COS(angles[1]) >> W2V_SHIFT;
								dest.x = src.x + (c * SIN(item->pos.yRot) >> W2V_SHIFT);
								dest.y = src.y + (8192 * SIN(-angles[1]) >> W2V_SHIFT);
								dest.z = src.z + (c * COS(item->pos.yRot) >> W2V_SHIFT);

								/*dest.x = LaserHeadData.target.x;
								dest.y = LaserHeadData.target.y;
								dest.z = LaserHeadData.target.z;*/

								if (item->itemFlags[3] != 90
									&& LaserHeadData.fireArcs[j] != NULL)
								{
									// Eye is aready firing
									SoundEffect(SFX_GOD_HEAD_LASER_LOOPS, &item->pos, 0);

									LaserHeadData.fireArcs[j]->pos1.x = src.x;
									LaserHeadData.fireArcs[j]->pos1.y = src.y;
									LaserHeadData.fireArcs[j]->pos1.z = src.z;
								}
								else
								{
									// Start firing from eye
									src.roomNumber = item->roomNumber;
									LaserHeadData.LOS = LOS(&src, &dest);
									LaserHeadData.fireArcs[j] = TriggerEnergyArc((PHD_VECTOR*)& src, (PHD_VECTOR*)& dest, 128, g, b, 32, 64, 64, ENERGY_ARC_NO_RANDOMIZE, ENERGY_ARC_STRAIGHT_LINE); // (GetRandomControl() & 7) + 4, b | ((&unk_640000 | g) << 8), 12, 64, 5);
									StopSoundEffect(SFX_GOD_HEAD_CHARGE);
									SoundEffect(SFX_GOD_HEAD_BLAST, &item->pos, 0);
								}

								ENERGY_ARC* currentArc = LaserHeadData.fireArcs[j];

								if (GlobalCounter & 1) 
								{
									TriggerGuardianSparks((PHD_VECTOR*)& src, 3, 0, g, b, 0);
									TriggerLightningGlow(src.x, src.y, src.z, (GetRandomControl() & 3) + 32, 0, g, b);
									TriggerDynamicLight(src.x, src.y, src.z, (GetRandomControl() & 3) + 16, 0, g, b);

									if (!LaserHeadData.LOS)
									{
										TriggerLightningGlow(currentArc->pos4.x, currentArc->pos4.y, currentArc->pos4.z, (GetRandomControl() & 3) + 16, 0, g, b);
										TriggerDynamicLight(currentArc->pos4.x, currentArc->pos4.y, currentArc->pos4.z, (GetRandomControl() & 3) + 6, 0, g, b);
										TriggerGuardianSparks((PHD_VECTOR*)& currentArc->pos4, 3, 0, g, b, 0);
									}
								}

								// Check if Lara was hit by energy arcs
								if (!Lara.burn)
								{
									int someIndex = 0;

									short* bounds = GetBoundsAccurate(LaraItem);
									short tbounds[6];

									phd_PushUnitMatrix();
									phd_RotYXZ(LaraItem->pos.yRot, LaraItem->pos.xRot, LaraItem->pos.zRot);
									phd_SetTrans(0, 0, 0);
									phd_RotBoundingBoxNoPersp(bounds, tbounds);
									phd_PopMatrix();

									int x1 = LaraItem->pos.xPos + tbounds[0];
									int x2 = LaraItem->pos.xPos + tbounds[1];
									int y1 = LaraItem->pos.yPos + tbounds[2];
									int y2 = LaraItem->pos.yPos + tbounds[3];
									int z1 = LaraItem->pos.zPos + tbounds[4];
									int z2 = LaraItem->pos.zPos + tbounds[5];

									int xc = LaraItem->pos.xPos + ((bounds[0] + bounds[1]) >> 1);
									int yc = LaraItem->pos.yPos + ((bounds[2] + bounds[3]) >> 1);
									int zc = LaraItem->pos.zPos + ((bounds[4] + bounds[5]) >> 1);

									int distance = SQRT_ASM(SQUARE(xc - src.x) + SQUARE(yc - src.y) + SQUARE(zc - src.z));

									if (distance < 8192)
									{
										int dl = distance + 512;

										if (dl < 8192)
										{
											dest.x = src.x + dl * (dest.x - src.x) / 8192;
											dest.y = src.y + dl * (dest.y - src.y) / 8192;
											dest.z = src.z + dl * (dest.z - src.z) / 8192;
										}

										int dx = (dest.x - src.x) >> 5;
										int dy = (dest.y - src.y) >> 5;
										int dz = (dest.z - src.z) >> 5;

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
												LaraBurn();
												Lara.BurnCount = 48;
												Lara.burnBlue = 2;
												LaraItem->hitPoints = 0;
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
							else if (item->itemFlags[3] > 90)
							{
								if (LaserHeadData.fireArcs[j])
								{
									LaserHeadData.fireArcs[j]->life = 0;
									LaserHeadData.fireArcs[j] = NULL;
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
		if (item->itemFlags[2] >= 8)
		{
			if (item->pos.yPos <= item->itemFlags[1])
			{
				src.x = 0;
				src.y = 168;
				src.z = 248;
				src.roomNumber = item->roomNumber;
				GetJointAbsPosition(item, (PHD_VECTOR*)& src, 0);

				dest.x = 0;
				dest.y = 0;
				dest.z = 0;
				GetJointAbsPosition(LaraItem, (PHD_VECTOR*)& dest, LJ_LHAND);

				if (LOS(&src, &src))
				{
					item->itemFlags[0]++;
					item->itemFlags[1] = item->pos.yPos;
					item->itemFlags[2] = 2640;
				}
			}
			else
			{
				item->fallspeed += 3;
				if (item->fallspeed > 32)
					item->fallspeed = 32;
				item->pos.yPos -= item->fallspeed;
			}
		}
		else if (!(GlobalCounter & 7))
		{
			short tentacleItemNumber = creature->tentacles[item->itemFlags[2]];
			ITEM_INFO* tentacleItem = &Items[tentacleItemNumber];
			AddActiveItem(tentacleItemNumber);
			tentacleItem->status = ITEM_ACTIVE;
			tentacleItem->flags |= 0x3E00;
			item->itemFlags[2]++;
		}
	}

	if (item->itemFlags[0] < 3)
	{
		int i = 0;

		/*for (i = 0; i < 8; i++)
		{
			short tentacleItemNumber = creature->tentacles[item->itemFlags[2]];
			ITEM_INFO* tentacleItem = &Items[tentacleItemNumber];
			if (tentacleItem->animNumber == Objects[tentacleItem->objectNumber].animIndex
				&& tentacleItem->frameNumber != Anims[tentacleItem->animNumber].frameEnd)
			{
				break;
			}
		}*/

		if (i == 8 && !(item->meshBits & 6))
		{
			if (LaserHeadData.fireArcs[0])
				LaserHeadData.fireArcs[0]->life = 2;
			if (LaserHeadData.fireArcs[1])
				LaserHeadData.fireArcs[1]->life = 2;

			LaserHeadData.fireArcs[0] = NULL;
			LaserHeadData.fireArcs[1] = NULL;

			item->itemFlags[0] = 3;
			item->itemFlags[3] = item->pos.yRot + (GetRandomControl() & 0x1000) - 2048;
			item->speed = 3;
			item->triggerFlags = item->pos.xRot + (GetRandomControl() & 0x1000) - 2048;
		}
	}
}

void TriggerGuardianSparks(PHD_VECTOR* pos, int count, byte r, byte g, byte b, int unk)
{
	if (count > 0)
	{
		for (int i = 0; i < count; i++)
		{
			SPARKS* spark = &Sparks[GetFreeSpark()];

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
			spark->transType = COLADD;
			spark->x = pos->x;
			spark->y = pos->y;
			spark->z = pos->z;
			spark->gravity = (GetRandomControl() >> 7) & 0x1F;
			spark->yVel = ((GetRandomControl() & 0xFFF) - 2048) << unk;
			spark->xVel = ((GetRandomControl() & 0xFFF) - 2048) << unk;
			spark->zVel = ((GetRandomControl() & 0xFFF) - 2048) << unk;
			spark->flags = 0;
			spark->maxYvel = 0;
			spark->friction = 34 << unk;
		}
	}
}

void GuardianCharge(ITEM_INFO* item)
{	
	byte size = item->itemFlags[3];
	byte g = ((GetRandomControl() & 0x1F) + 128);
	byte b = ((GetRandomControl() & 0x1F) + 64);

	if (item->itemFlags[3] <= 32)
	{
		g = item->itemFlags[3] * g >> 5;
		b = item->itemFlags[3] * b >> 5;
	}
	else
	{
		size = 32;
	}

	LASER_HEAD_INFO* creature = (LASER_HEAD_INFO*)item->data;
	
	PHD_VECTOR src, dest;
	dest.x = LaserHeadBasePosition.x;
	dest.y = LaserHeadBasePosition.y;
	dest.z = LaserHeadBasePosition.z;
	GetJointAbsPosition(&Items[creature->baseItem], &dest, 0);

	for (int i = 0; i < 4; i++)
	{
		ENERGY_ARC* arc = LaserHeadData.chargeArcs[i];

		if (item->itemFlags[3] & 0x0F && arc != NULL)
		{
			arc->r = 0;
			arc->g = g;
			arc->b = b;
			//arc->segmentSize = 50;
		}
		else
		{
			src.x = GuardianChargePositions[i].x;
			src.y = GuardianChargePositions[i].y;
			src.z = GuardianChargePositions[i].z;
			GetJointAbsPosition(&Items[creature->baseItem], &src, 0);
			//LaserHeadData.chargeArcs[i] = TriggerEnergyArc(&src, &dest, 255, 255, 255, 256, 90, 64, ENERGY_ARC_STRAIGHT_LINE); //  (GetRandomControl() & 7) + 8, v4 | ((v1 | 0x240000) << 8), 13, 48, 3);
		}
	}

	if (GlobalCounter & 1)
	{
		for (int i = 0; i < 5; i += 4)
		{
			if (2 * GuardianMeshes[i] & item->meshBits)
			{
				src.x = 0;
				src.y = 0;
				src.z = 0;
				GetJointAbsPosition(item, &src, GuardianMeshes[i]);

				TriggerLightningGlow(src.x, src.y, src.z, size + (GetRandomControl() & 3), 0, g, b);
				TriggerGuardianSparks(&src, 3, 0, g, b, 0);
			}
		}

		TriggerLightningGlow(dest.x, dest.y, dest.z, (GetRandomControl() & 3) + size + 8, 0, g, b);
		TriggerDynamicLight(dest.x, dest.y, dest.z, (GetRandomControl() & 3) + 16, 0, g, b);
	}

	if (!(GlobalCounter & 3))
	{
		TriggerEnergyArc(&dest, (PHD_VECTOR*)&item->pos, 0, g, b, 256, 3, 64, ENERGY_ARC_NO_RANDOMIZE, ENERGY_ARC_STRAIGHT_LINE);
		//TriggerEnergyArc(&dest, &item->pos, (GetRandomControl() & 7) + 8, v4 | ((v1 | 0x180000) << 8), 13, 64, 3);
	}

	TriggerGuardianSparks(&dest, 3, 0, g, b, 1);
}