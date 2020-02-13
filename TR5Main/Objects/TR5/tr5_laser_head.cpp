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

struct LASER_HEAD_INFO
{
	short baseItem;
	short arcsItems[8];
	short puzzleItem;
};

struct LASER_HEAD_STRUCT
{
	PHD_VECTOR pos;
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
PHD_VECTOR LaserHeadChargePositions[8];
int LaserHeadUnknownArray[5] = { 1,0,0,0,2 };

void InitialiseLaserHead(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	item->data = (LASER_HEAD_INFO*)GameMalloc(sizeof(LASER_HEAD_INFO));
	LASER_HEAD_INFO* info = (LASER_HEAD_INFO*)item->data;

	for (int i = 0; i < LevelItems; i++)
	{
		if (Items[i].objectNumber == ID_ANIMATING12)
		{
			info->baseItem = i;
			break;
		}
	}

	short rotation = 0;
	int j = 0;
	for (int i = 0; i < LevelItems; i++)
	{
		// ID_WINGED_MUMMY beccause we recycled MIP slots
		if (Items[i].objectNumber == ID_WINGED_MUMMY && Items[i].pos.yRot == rotation)
		{
			info->arcsItems[j] = i;
			rotation += ANGLE(45);
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

	item->pos.yPos -= 640;
	item->itemFlags[1] = item->pos.yPos - 640;
	item->currentAnimState = 0;
	item->itemFlags[3] = 90;

	ZeroMemory(&LaserHeadData, sizeof(LASER_HEAD_STRUCT));
}

void ControlLaserHead(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	LASER_HEAD_INFO* creature = (LASER_HEAD_INFO*)item->data;

	GAME_VECTOR src, dest;

	if (item->itemFlags[0])
	{
		// Maybe number of eye hits?
		if (item->itemFlags[0] > 2)
		{
			if (!(GlobalCounter & 7))
			{
				if (item->currentAnimState < 8)
				{
					short arcItemNumber = creature->arcsItems[item->currentAnimState];
					Items[arcItemNumber].goalAnimState = 2;
					item->currentAnimState++;
				}
			}

			// Destroy arcs items
			if (item->currentAnimState > 0)
			{
				for (int i = 0; i < 8; i++)
				{
					ITEM_INFO* arcItem = &Items[creature->arcsItems[i]];

					if (arcItem->animNumber == Objects[arcItem->objectNumber].animIndex + 1
						&& arcItem->frameNumber == Anims[arcItem->animNumber].frameEnd
						&& arcItem->meshBits & 1)
					{
						SoundEffect(SFX_SMASH_ROCK, &item->pos, 0);
						ExplodeItemNode(arcItem, 0, 0, 128);
						KillItem(creature->arcsItems[i]);
					}
				}
			}

			item->pos.yPos = item->itemFlags[1] - ((192 - item->speed) * SIN(item->itemFlags[2]) >> W2V_SHIFT);
			item->itemFlags[2] += ANGLE(item->speed);

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
				//TriggerShockwave(&item->pos, &unk_A00020, 64, 604012608, 0);
				//TriggerShockwave(&item->pos, &unk_A00020, 64, 604012608, 12288);
				//TriggerShockwave(&item->pos, &unk_A00020, 64, 604012608, 24576);

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
			item->pos.yPos = item->itemFlags[1] - 128 * SIN(item->itemFlags[2]) >> W2V_SHIFT;
			item->itemFlags[2] += ANGLE(3);

			src.x = 0;
			src.y = 168;
			src.z = 248;
			src.roomNumber = item->roomNumber;
			GetJointAbsPosition(item, (PHD_VECTOR*)& src, 0);

			if (item->itemFlags[0] == 1)
			{
				dest.x = 0;
				dest.y = 0;
				dest.z = 0;
				GetJointAbsPosition(LaraItem, (PHD_VECTOR*)& dest, 14);

				int distance = SQRT_ASM(SQUARE(src.x - dest.x) + SQUARE(src.y - dest.y) + SQUARE(src.z - dest.z));

				if (LOS(&src, &dest)
					&& distance <= 8192
					&& LaraItem->hitPoints > 0
					&& !Lara.burn
					&& (LaserHeadData.pos.x || LaserHeadData.pos.y || LaserHeadData.pos.z))
				{
					dest.x = 0;
					dest.y = 0;
					dest.z = 0;
					GetJointAbsPosition(LaraItem, (PHD_VECTOR*)& dest, 0);

					LaserHeadData.pos.x = dest.x;
					LaserHeadData.pos.y = dest.y;
					LaserHeadData.pos.z = dest.z;
					LaserHeadData.byte1 = 3;
					LaserHeadData.byte2 = 1;
				}
				else
				{
					bool condition = !(GetRandomControl() & 0x7F) && item->triggerFlags > 150;
					item->itemFlags[3]--;

					if (item->itemFlags[3] <= 0 || condition)
					{
						short angle1 = (GetRandomControl() >> 2) - 4096;
						short angle2;
						if (condition)
							angle2 = item->pos.yRot + (GetRandomControl() & 0x3FFF) + 24576;
						else
							angle2 = 2 * GetRandomControl();
						int v = ((GetRandomControl() & 0x1FFF) + 0x2000);
						int c = v * COS(-angle1) >> W2V_SHIFT;
						dest.x = src.x + (c * SIN(angle2) >> W2V_SHIFT);
						dest.y = src.y + (v * SIN(-angle1) >> W2V_SHIFT);
						dest.z = src.z + (c * COS(angle2) >> W2V_SHIFT);

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

						LaserHeadData.pos.x = dest.x;
						LaserHeadData.pos.y = dest.y;
						LaserHeadData.pos.z = dest.z;
					}
					else
					{
						dest.x = LaserHeadData.pos.x;
						dest.y = LaserHeadData.pos.y;
						dest.z = LaserHeadData.pos.z;
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

					dest.x = LaserHeadData.pos.x = src.x + (c * SIN(item->pos.yRot) >> W2V_SHIFT);
					dest.y = LaserHeadData.pos.y = src.y + 8192 * SIN(3328 - item->pos.xRot) >> W2V_SHIFT;
					dest.z = LaserHeadData.pos.z = src.z + (c * COS(item->pos.yRot) >> W2V_SHIFT);
				}
				else
				{
					dest.x = LaserHeadData.pos.x;
					dest.y = LaserHeadData.pos.y;
					dest.z = LaserHeadData.pos.z;
				}
			}

			short angles[2];
			short outAngle;
			phd_GetVectorAngles(LaserHeadData.pos.x - src.x, LaserHeadData.pos.y - src.y, LaserHeadData.pos.z - src.z, angles);
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
				if (item->itemFlags[3] <= 90)
				{
					SoundEffect(SFX_GOD_HEAD_CHARGE, &item->pos, 0);
					LaserHeadCharge(item);
					item->itemFlags[3]++;
				}

				if (item->itemFlags[3] > 90)
				{
					byte r, g, b;

					g = (GetRandomControl() & 0x1F) + 128;
					b = (GetRandomControl() & 0x1F) + 64;

					ENERGY_ARC* arc = LaserHeadData.fireArcs[0];
					if (!LaserHeadData.fireArcs[0])
						arc = LaserHeadData.fireArcs[1];

					if (item->itemFlags[3] > 90 && arc && !arc->life || LaraItem->hitPoints <= 0 || Lara.burn)
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
						if (item->itemFlags[3] > 90 && arc && arc->life < 16)
						{
							g = b = arc->life * g >> 4;
						}

						for (int i = 0; i < 2; i++)
						{
							if (2 * LaserHeadUnknownArray[i] & item->meshBits)
							{
								src.x = 0;
								src.y = 0;
								src.z = 0;
								GetJointAbsPosition(item, (PHD_VECTOR*)& src, LaserHeadUnknownArray[i]);

								int c = 8192 * COS(angles[1]) >> W2V_SHIFT;
								dest.x = src.x + (c * SIN(item->pos.yRot) >> W2V_SHIFT);
								dest.y = src.y + 8192 * COS(-angles[1]) >> W2V_SHIFT;
								dest.z = src.z + (c * COS(item->pos.yRot) >> W2V_SHIFT);

								if (item->itemFlags[3] != 90 && LaserHeadData.fireArcs[i] != NULL)
								{
									SoundEffect(SFX_GOD_HEAD_LASER_LOOPS, &item->pos, 0);

									LaserHeadData.fireArcs[i]->pos1.x = src.x;
									LaserHeadData.fireArcs[i]->pos1.y = src.y;
									LaserHeadData.fireArcs[i]->pos1.z = src.z;
								}
								else
								{
									src.roomNumber = item->roomNumber;
									LaserHeadData.LOS = LOS(&src, &dest);
									//LaserHeadData.fireArcs[i] = TriggerEnergyArc(&src, &dest, (GetRandomControl() & 7) + 4, b | ((&unk_640000 | g) << 8), 12, 64, 5);
									StopSoundEffect(SFX_GOD_HEAD_CHARGE);
									SoundEffect(SFX_GOD_HEAD_BLAST, &item->pos, 0);
								}

								ENERGY_ARC* currentArc = LaserHeadData.fireArcs[i];

								if (GlobalCounter & 1)
								{
									TriggerLaserHeadSparks1((PHD_VECTOR*)& src, 3, (CVECTOR*)(b | (g << 8)), 0);
									TriggerLightningGlow(src.x, src.y, src.z, b | (g << 8) | (((GetRandomControl() & 3) + 32) << 24));
									TriggerDynamicLight(src.x, src.y, src.z, (GetRandomControl() & 3) + 16, 0, g, b);

									if (!LaserHeadData.LOS)
									{
										TriggerLightningGlow(currentArc->pos4.x, currentArc->pos4.y, currentArc->pos4.z, b | g | (((GetRandomControl() & 3) + 16) << 24));
										TriggerDynamicLight(currentArc->pos4.x, currentArc->pos4.y, currentArc->pos4.z, (GetRandomControl() & 3) + 6, 0, g, b);
										TriggerLaserHeadSparks1((PHD_VECTOR*)&currentArc->pos4, 3, (CVECTOR*)(b | (g << 8)), 0);
									}
								}

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
				item->pos.yPos = item->pos.yPos - item->fallspeed;
			}
		}
		else if (!(GlobalCounter & 7))
		{
			short arcItemNumber = creature->arcsItems[item->itemFlags[2]];
			ITEM_INFO* arcItem = &Items[arcItemNumber];
			AddActiveItem(arcItemNumber);
			arcItem->status = ITEM_ACTIVE;
			arcItem->flags |= 0x3E00;
			item->itemFlags[2]++;
		}
	}

	if (item->itemFlags[0] < 3)
	{
		int i;

		for (i = 0; i < 8; i++)
		{
			short arcItemNumber = creature->arcsItems[item->itemFlags[2]];
			ITEM_INFO* arcItem = &Items[arcItemNumber];
			if (arcItem->animNumber == Objects[arcItem->objectNumber].animIndex
				&& arcItem->frameNumber != Anims[arcItem->animNumber].frameEnd)
			{
				break;
			}
		}

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

void TriggerLaserHeadSparks1(PHD_VECTOR* pos, int count, CVECTOR* color, int unk)
{
	if (count > 0)
	{
		for (int i = 0; i < count; i++)
		{
			SPARKS* spark = &Sparks[GetFreeSpark()];

			spark->on = 1;
			spark->sR = color->r;
			spark->sG = color->g;
			spark->sB = color->b;
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

void LaserHeadCharge(ITEM_INFO* item)
{	
	byte v1, v4;
	int flags = item->itemFlags[3];

	if (item->itemFlags[3] <= 32)
	{
		v1 = item->itemFlags[3] * ((GetRandomControl() & 0x1F) + 128) >> 5;
		v4 = item->itemFlags[3] * ((GetRandomControl() & 0x1F) + 64) >> 5;
	}
	else
	{
		flags = 32;
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
			*&(arc->r) = v4 | ((v1 | 0x320000) << 8);
		}
		else
		{
			src.x = LaserHeadChargePositions[i].x;
			src.y = LaserHeadChargePositions[i].y;
			src.z = LaserHeadChargePositions[i].z;
			GetJointAbsPosition(&Items[creature->baseItem], &src, 0);
			//LaserHeadData.chargeArcs[i] = TriggerEnergyArc(&src, &dest, (GetRandomControl() & 7) + 8, v4 | ((v1 | 0x240000) << 8), 13, 48, 3);
		}
	}

	if (GlobalCounter & 1)
	{
		for (int i = 0; i < 2; i++)
		{
			if (2 * LaserHeadUnknownArray[i] & item->meshBits)
			{
				src.x = 0;
				src.y = 0;
				src.z = 0;
				GetJointAbsPosition(item, &src, LaserHeadUnknownArray[i]);

				TriggerLightningGlow(src.x, src.y, src.z, v4 | (v1 << 8) | ((flags + (GetRandomControl() & 3)) << 24));
				TriggerLaserHeadSparks1(&src, 3, (CVECTOR*)(v4 | (v1 << 8)), 0);
			}
		}

		TriggerLightningGlow(dest.x, dest.y, dest.z, v4 | (v1 << 8) | (((GetRandomControl() & 3) + flags + 8) << 24));
		TriggerDynamicLight(dest.x, dest.y, dest.z, (GetRandomControl() & 3) + 16, 0, v1, v4);
	}

	if (!(GlobalCounter & 3))
	{
		//TriggerEnergyArc(&dest, &item->pos, (GetRandomControl() & 7) + 8, v4 | ((v1 | 0x180000) << 8), 13, 64, 3);
	}

	TriggerLaserHeadSparks1(&dest, 3, (CVECTOR*)(v4 | (v1 << 8)), 1);
}