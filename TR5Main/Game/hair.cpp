#include "hair.h"
#include "..\Global\global.h"
#include "draw.h"
#include "laramisc.h"
#include "..\Scripting\GameFlowScript.h"
#include "../Specific/setup.h"

int FirstHair[2];
HAIR_STRUCT Hairs[2][7];
int WindAngle;
int DWindAngle;
int Wind;

extern GameFlow* g_GameFlow;

void InitialiseHair()
{
	for (int h = 0; h < 2; h++)
	{
		FirstHair[h] = 1;

		int* bone = Bones + Objects[ID_LARA_HAIR].boneIndex;

		Hairs[h][0].pos.yRot = 0;
		Hairs[h][0].pos.xRot = -0x4000;

		for (int i = 1; i < HAIR_SEGMENTS; i++, bone += 4)
		{
			Hairs[h][i].pos.xPos = *(bone + 1);
			Hairs[h][i].pos.yPos = *(bone + 2);
			Hairs[h][i].pos.zPos = *(bone + 3);
			Hairs[h][i].pos.xRot = -0x4000;
			Hairs[h][i].pos.yRot = Hairs[h][i].pos.zRot = 0;

			Hairs[h][i].hvel.x = Hairs[h][i].hvel.y = Hairs[h][i].hvel.z = 0;
		}
	}
}


void HairControl(int cutscene, int ponytail, short* framePtr)
{
	SPHERE sphere[5];
	OBJECT_INFO* object = &Objects[ID_LARA];
	short* frame;
	int spaz;
	bool youngLara = g_GameFlow->GetLevel(CurrentLevel)->LaraType == LARA_YOUNG;

	if (framePtr == NULL)
	{
		if (Lara.hitDirection >= 0)
		{
			switch (Lara.hitDirection)
			{
			case NORTH:
				if (Lara.isDucked)
					spaz = 294;
				else
					spaz = 125;
				break;

			case SOUTH:
				if (Lara.isDucked)
					spaz = 293;
				else
					spaz = 126;
				break;

			case EAST:
				if (Lara.isDucked)
					spaz = 295;
				else
					spaz = 127;
				break;

			default:
				if (Lara.isDucked)
					spaz = 296;
				else
					spaz = 128;
				break;
			}

			frame = Anims[spaz].framePtr;
			int size = Anims[spaz].interpolation >> 8;

			frame += (int)(Lara.hitFrame * size);
		}
		else
			frame = GetBestFrame(LaraItem);
	}
	else
	{
		frame = framePtr;
	}

	phd_PushUnitMatrix();

	*(MatrixPtr + M03) = LaraItem->pos.xPos << W2V_SHIFT;
	*(MatrixPtr + M13) = LaraItem->pos.yPos << W2V_SHIFT;
	*(MatrixPtr + M23) = LaraItem->pos.zPos << W2V_SHIFT;
	phd_RotYXZ(LaraItem->pos.yRot, LaraItem->pos.xRot, LaraItem->pos.zRot);

	short* rotation = frame + 9;
	int* bone = Bones + object->boneIndex;

	phd_TranslateRel((int) * (frame + 6), (int) * (frame + 7), (int) * (frame + 8));
	gar_RotYXZsuperpack(&rotation, 0);

	// Hips
	phd_PushMatrix();
	short* objptr = Lara.meshPtrs[LM_HIPS];
	phd_TranslateRel(*objptr, *(objptr + 1), *(objptr + 2));
	sphere[0].x = *(MatrixPtr + M03) >> W2V_SHIFT;
	sphere[0].y = *(MatrixPtr + M13) >> W2V_SHIFT;
	sphere[0].z = *(MatrixPtr + M23) >> W2V_SHIFT;
	sphere[0].r = (int) * (objptr + 3);
	phd_PopMatrix();

	phd_TranslateRel(*(bone + 1 + 24), *(bone + 2 + 24), *(bone + 3 + 24));

	gar_RotYXZsuperpack(&rotation, 6);
	phd_RotYXZ(Lara.torsoYrot, Lara.torsoXrot, Lara.torsoZrot);

	// Torso
	phd_PushMatrix();
	objptr = Lara.meshPtrs[LM_TORSO];
	phd_TranslateRel(*objptr, *(objptr + 1), *(objptr + 2));
	sphere[1].x = *(MatrixPtr + M03) >> W2V_SHIFT;
	sphere[1].y = *(MatrixPtr + M13) >> W2V_SHIFT;
	sphere[1].z = *(MatrixPtr + M23) >> W2V_SHIFT;
	sphere[1].r = (int) * (objptr + 3);
	if (youngLara)
		sphere[1].r = sphere[1].r - ((sphere[1].r >> 2) + (sphere[1].r >> 3));
	phd_PopMatrix();

	phd_PushMatrix();
	phd_TranslateRel(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));
	gar_RotYXZsuperpack(&rotation, 0);

	// Right arm
	objptr = Lara.meshPtrs[LM_RINARM];
	phd_TranslateRel(*objptr, *(objptr + 1), *(objptr + 2));
	sphere[3].x = *(MatrixPtr + M03) >> W2V_SHIFT;
	sphere[3].y = *(MatrixPtr + M13) >> W2V_SHIFT;
	sphere[3].z = *(MatrixPtr + M23) >> W2V_SHIFT;
	sphere[3].r = (int) * (objptr + 3) * 3 / 2; // 1.5 times sphere size gives better result
	phd_PopMatrix();

	phd_PushMatrix();
	phd_TranslateRel(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));
	gar_RotYXZsuperpack(&rotation, 2);

	// Left arm
	objptr = Lara.meshPtrs[LM_LINARM];
	phd_TranslateRel(*objptr, *(objptr + 1), *(objptr + 2));
	sphere[4].x = *(MatrixPtr + M03) >> W2V_SHIFT;
	sphere[4].y = *(MatrixPtr + M13) >> W2V_SHIFT;
	sphere[4].z = *(MatrixPtr + M23) >> W2V_SHIFT;
	sphere[4].r = (int) * (objptr + 3) * 3 / 2;
	phd_PopMatrix();

	phd_TranslateRel(*(bone + 1 + 52), *(bone + 2 + 52), *(bone + 3 + 52));
	gar_RotYXZsuperpack(&rotation, 2);
	phd_RotYXZ(Lara.headYrot, Lara.headXrot, Lara.headZrot);

	// Head
	phd_PushMatrix();
	objptr = Lara.meshPtrs[LM_HEAD];
	phd_TranslateRel(*objptr, *(objptr + 1), *(objptr + 2));
	sphere[2].x = *(MatrixPtr + M03) >> W2V_SHIFT;
	sphere[2].y = *(MatrixPtr + M13) >> W2V_SHIFT;
	sphere[2].z = *(MatrixPtr + M23) >> W2V_SHIFT;
	sphere[2].r = (int) * (objptr + 3);
	phd_PopMatrix();

	if (youngLara)
	{
		sphere[1].x = (sphere[1].x + sphere[2].x) / 2;
		sphere[1].y = (sphere[1].y + sphere[2].y) / 2;
		sphere[1].z = (sphere[1].z + sphere[2].z) / 2;
	}

	if (ponytail)
	{
		phd_TranslateRel(44, -48, -50);
	}
	else if (youngLara)
	{
		phd_TranslateRel(-52, -48, -50);
	}
	else
	{
		phd_TranslateRel(-4, -48, -48);
	}

	PHD_VECTOR pos;
	pos.x = *(MatrixPtr + M03) >> W2V_SHIFT;
	pos.y = *(MatrixPtr + M13) >> W2V_SHIFT;
	pos.z = *(MatrixPtr + M23) >> W2V_SHIFT;

	phd_PopMatrix();

	bone = bone + Objects[ID_LARA_HAIR].boneIndex;

	if (FirstHair[ponytail])
	{
		FirstHair[ponytail] = 0;

		Hairs[ponytail][0].pos.xPos = pos.x;
		Hairs[ponytail][0].pos.yPos = pos.y;
		Hairs[ponytail][0].pos.zPos = pos.z;

		for (int i = 0; i < HAIR_SEGMENTS - 1; i++, bone += 4)
		{
			phd_PushUnitMatrix();

			*(MatrixPtr + M03) = Hairs[ponytail][i].pos.xPos << W2V_SHIFT;
			*(MatrixPtr + M13) = Hairs[ponytail][i].pos.yPos << W2V_SHIFT;
			*(MatrixPtr + M23) = Hairs[ponytail][i].pos.zPos << W2V_SHIFT;

			phd_RotYXZ(Hairs[ponytail][i].pos.yRot, Hairs[ponytail][i].pos.xRot, 0);
			phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));

			Hairs[ponytail][i + 1].pos.xPos = *(MatrixPtr + M03) >> W2V_SHIFT;
			Hairs[ponytail][i + 1].pos.yPos = *(MatrixPtr + M13) >> W2V_SHIFT;
			Hairs[ponytail][i + 1].pos.zPos = *(MatrixPtr + M23) >> W2V_SHIFT;

			phd_PopMatrix();
		}

		Wind = SmokeWindX = SmokeWindZ = 0;
		WindAngle = DWindAngle = 2048;
	}
	else
	{
		Hairs[ponytail][0].pos.xPos = pos.x;
		Hairs[ponytail][0].pos.yPos = pos.y;
		Hairs[ponytail][0].pos.zPos = pos.z;

		short roomNumber = LaraItem->roomNumber;
		int wh;

		if (cutscene)
			wh = NO_HEIGHT;
		else
		{
			int x = LaraItem->pos.xPos + (frame[0] + frame[1]) / 2;
			int y = LaraItem->pos.yPos + (frame[2] + frame[3]) / 2;
			int z = LaraItem->pos.zPos + (frame[4] + frame[5]) / 2;
			wh = GetWaterHeight(x, y, z, roomNumber);
		}

		Wind += (GetRandomControl() & 7) - 3;
		if (Wind <= -2)
			Wind++;
		else if (Wind >= 9)
			Wind--;

		DWindAngle = (DWindAngle + 2 * (GetRandomControl() & 63) - 64) & 0x1FFE;

		if (DWindAngle < 1024)
			DWindAngle = 2048 - DWindAngle;
		else if (DWindAngle > 3072)
			DWindAngle += 6144 - 2 * DWindAngle;

		WindAngle = (WindAngle + ((DWindAngle - WindAngle) >> 3)) & 0x1FFE;

		SmokeWindX = Wind * rcossin_tbl[WindAngle] >> 12;
		SmokeWindZ = Wind * rcossin_tbl[WindAngle + 1] >> 12;

		SmokeWindX = (((rcossin_tbl[WindAngle]) * Wind) >> 12);
		SmokeWindZ = (((rcossin_tbl[WindAngle + 1]) * Wind) >> 12);

		for (int i = 1; i < HAIR_SEGMENTS; i++, bone += 4)
		{
			Hairs[ponytail][0].hvel.x = Hairs[ponytail][i].pos.xPos;
			Hairs[ponytail][0].hvel.y = Hairs[ponytail][i].pos.yPos;
			Hairs[ponytail][0].hvel.z = Hairs[ponytail][i].pos.zPos;

			int height;

			if (!cutscene)
			{
				FLOOR_INFO* floor = GetFloor(Hairs[ponytail][i].pos.xPos, Hairs[ponytail][i].pos.yPos, Hairs[ponytail][i].pos.zPos, &roomNumber);
				height = GetFloorHeight(floor, Hairs[ponytail][i].pos.xPos, Hairs[ponytail][i].pos.yPos, Hairs[ponytail][i].pos.zPos);
			}
			else
				height = 32767;

			Hairs[ponytail][i].pos.xPos += Hairs[ponytail][i].hvel.x * 3 / 4;
			Hairs[ponytail][i].pos.yPos += Hairs[ponytail][i].hvel.y * 3 / 4;
			Hairs[ponytail][i].pos.zPos += Hairs[ponytail][i].hvel.z * 3 / 4;

			if (Lara.waterStatus == LW_ABOVE_WATER && Rooms[roomNumber].flags & ENV_FLAG_WIND)
			{
				Hairs[ponytail][i].pos.xPos += SmokeWindX;
				Hairs[ponytail][i].pos.zPos += SmokeWindZ;
			}

			switch (Lara.waterStatus)
			{
			case LW_ABOVE_WATER:
				Hairs[ponytail][i].pos.yPos += 10;
				if (wh != NO_HEIGHT && Hairs[ponytail][i].pos.yPos > wh)
					Hairs[ponytail][i].pos.yPos = wh;
				else if (Hairs[ponytail][i].pos.yPos > height)
				{
					Hairs[ponytail][i].pos.xPos = Hairs[ponytail][0].hvel.x;
					Hairs[ponytail][i].pos.zPos = Hairs[ponytail][0].hvel.z;
				}
				break;

			case LW_UNDERWATER:
			case LW_SURFACE:
			case LW_WADE:
				if (Hairs[ponytail][i].pos.yPos < wh)
					Hairs[ponytail][i].pos.yPos = wh;
				else if (Hairs[ponytail][i].pos.yPos > height)
					Hairs[ponytail][i].pos.yPos = height;
				break;
			}

			for (int j = 0; j < 5; j++)
			{
				int x = Hairs[ponytail][i].pos.xPos - sphere[j].x;
				int y = Hairs[ponytail][i].pos.yPos - sphere[j].y;
				int z = Hairs[ponytail][i].pos.zPos - sphere[j].z;

				int distance = SQUARE(x) + SQUARE(y) + SQUARE(z);

				if (distance < SQUARE(sphere[j].r))
				{
					distance = SQRT_ASM(distance);

					if (distance == 0)
						distance = 1;

					Hairs[ponytail][i].pos.xPos = sphere[j].x + x * sphere[j].r / distance;
					Hairs[ponytail][i].pos.yPos = sphere[j].y + y * sphere[j].r / distance;
					Hairs[ponytail][i].pos.zPos = sphere[j].z + z * sphere[j].r / distance;
				}
			}

			int distance = SQRT_ASM(SQUARE(Hairs[ponytail][i].pos.zPos - Hairs[ponytail][i - 1].pos.zPos) + SQUARE(Hairs[ponytail][i].pos.xPos - Hairs[ponytail][i - 1].pos.xPos));
			Hairs[ponytail][i - 1].pos.yRot = ATAN((Hairs[ponytail][i].pos.zPos - Hairs[ponytail][i - 1].pos.zPos), (Hairs[ponytail][i].pos.xPos - Hairs[ponytail][i - 1].pos.xPos));
			Hairs[ponytail][i - 1].pos.xRot = -ATAN(distance, Hairs[ponytail][i].pos.yPos - Hairs[ponytail][i - 1].pos.yPos);

			phd_PushUnitMatrix();

			*(MatrixPtr + M03) = Hairs[ponytail][i - 1].pos.xPos << W2V_SHIFT;
			*(MatrixPtr + M13) = Hairs[ponytail][i - 1].pos.yPos << W2V_SHIFT;
			*(MatrixPtr + M23) = Hairs[ponytail][i - 1].pos.zPos << W2V_SHIFT;
			phd_RotYXZ(Hairs[ponytail][i - 1].pos.yRot, Hairs[ponytail][i - 1].pos.xRot, 0);

			if (i == HAIR_SEGMENTS - 1)
				phd_TranslateRel(*(bone - 3), *(bone - 2), *(bone - 1));
			else
				phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));

			Hairs[ponytail][i].pos.xPos = *(MatrixPtr + M03) >> W2V_SHIFT;
			Hairs[ponytail][i].pos.yPos = *(MatrixPtr + M13) >> W2V_SHIFT;
			Hairs[ponytail][i].pos.zPos = *(MatrixPtr + M23) >> W2V_SHIFT;

			Hairs[ponytail][i].hvel.x = Hairs[ponytail][i].pos.xPos - Hairs[ponytail][0].hvel.x;
			Hairs[ponytail][i].hvel.y = Hairs[ponytail][i].pos.yPos - Hairs[ponytail][0].hvel.y;
			Hairs[ponytail][i].hvel.z = Hairs[ponytail][i].pos.zPos - Hairs[ponytail][0].hvel.z;

			phd_PopMatrix();
		}
	}
}

void Inject_Hair()
{
	
}