#include "framework.h"
#include "hair.h"

#include "animation.h"
#include "lara.h"
#include "control/control.h"
#include "GameFlowScript.h"
#include "setup.h"
#include "sphere.h"
#include "level.h"
#include "effects\weather.h"
#include "Renderer11.h"
#include "items.h"

using namespace TEN::Effects::Environment;
using TEN::Renderer::g_Renderer;

HAIR_STRUCT Hairs[HAIR_MAX][HAIR_SEGMENTS + 1];

void InitialiseHair()
{
	for (int h = 0; h < HAIR_MAX; h++)
	{
		int* bone = &g_Level.Bones[Objects[ID_LARA_HAIR].boneIndex];

		Hairs[h][0].initialized = true;
		Hairs[h][0].pos.yRot = 0;
		Hairs[h][0].pos.xRot = -0x4000;

		for (int i = 1; i < HAIR_SEGMENTS + 1; i++, bone += 4)
		{
			Hairs[h][i].initialized = true;
			Hairs[h][i].pos.xPos = *(bone + 1);
			Hairs[h][i].pos.yPos = *(bone + 2);
			Hairs[h][i].pos.zPos = *(bone + 3);
			Hairs[h][i].pos.xRot = -0x4000;
			Hairs[h][i].pos.yRot = Hairs[h][i].pos.zRot = 0;

			Hairs[h][i].hvel.x = Hairs[h][i].hvel.y = Hairs[h][i].hvel.z = 0;
		}
	}
}

void HairControl(int cutscene, int ponytail, ANIM_FRAME* framePtr)
{
	SPHERE sphere[HAIR_SPHERE];
	OBJECT_INFO* object = &Objects[ID_LARA];
	ANIM_FRAME* frame;
	int spaz;
	bool youngLara = g_GameFlow->GetLevel(CurrentLevel)->LaraType == LaraType::Young;

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

			frame = &g_Level.Frames[g_Level.Anims[spaz].framePtr + Lara.hitFrame];
		}
		else
			frame = GetBestFrame(LaraItem);
	}
	else
	{
		frame = framePtr;
	}

	// Get Lara's spheres in absolute coords, for head, torso, hips and upper arms
	MESH* mesh = &g_Level.Meshes[Lara.meshPtrs[LM_HIPS]];
	PHD_VECTOR pos = { (int)mesh->sphere.Center.x, (int)mesh->sphere.Center.y, (int)mesh->sphere.Center.z };
	GetLaraJointPosition(&pos, LM_HIPS);
	sphere[0].x = pos.x;
	sphere[0].y = pos.y;
	sphere[0].z = pos.z;
	sphere[0].r = (int)mesh->sphere.Radius;

	mesh = &g_Level.Meshes[Lara.meshPtrs[LM_TORSO]];
	pos = { (int)mesh->sphere.Center.x - 10, (int)mesh->sphere.Center.y, (int)mesh->sphere.Center.z + 25 }; // Repositioning sphere - from tomb5 
	GetLaraJointPosition(&pos, LM_TORSO);
	sphere[1].x = pos.x;
	sphere[1].y = pos.y;
	sphere[1].z = pos.z;
	sphere[1].r = (int)mesh->sphere.Radius;
	if (youngLara)
		sphere[1].r = sphere[1].r - ((sphere[1].r >> 2) + (sphere[1].r >> 3));

	mesh = &g_Level.Meshes[Lara.meshPtrs[LM_HEAD]];
	pos = { (int)mesh->sphere.Center.x - 2, (int)mesh->sphere.Center.y, (int)mesh->sphere.Center.z }; // Repositioning sphere - from tomb5 
	GetLaraJointPosition(&pos, LM_HEAD);
	sphere[2].x = pos.x;
	sphere[2].y = pos.y;
	sphere[2].z = pos.z;
	sphere[2].r = (int)mesh->sphere.Radius;

	mesh = &g_Level.Meshes[Lara.meshPtrs[LM_RINARM]];
	pos = { (int)mesh->sphere.Center.x, (int)mesh->sphere.Center.y, (int)mesh->sphere.Center.z };
	GetLaraJointPosition(&pos, LM_RINARM);
	sphere[3].x = pos.x;
	sphere[3].y = pos.y;
	sphere[3].z = pos.z;
	sphere[3].r = (int)(4.0f * mesh->sphere.Radius / 3.0f); // Resizing sphere - from tomb5 

	mesh = &g_Level.Meshes[Lara.meshPtrs[LM_LINARM]];
	pos = { (int)mesh->sphere.Center.x, (int)mesh->sphere.Center.y, (int)mesh->sphere.Center.z };
	GetLaraJointPosition(&pos, LM_LINARM);
	sphere[4].x = pos.x;
	sphere[4].y = pos.y;
	sphere[4].z = pos.z;
	sphere[4].r = (int)(4.0f * mesh->sphere.Radius / 3.0f); // Resizing sphere - from tomb5

	if (youngLara)
	{
		sphere[1].x = (sphere[1].x + sphere[2].x) / 2;
		sphere[1].y = (sphere[1].y + sphere[2].y) / 2;
		sphere[1].z = (sphere[1].z + sphere[2].z) / 2;
	}

	// Extra neck sphere - from tomb5
	sphere[5].x = (2 * sphere[2].x + sphere[1].x) / 3;
	sphere[5].y = (2 * sphere[2].y + sphere[1].y) / 3;
	sphere[5].z = (2 * sphere[2].z + sphere[1].z) / 3;
	sphere[5].r = youngLara ? 0 : (int)(3.0f * (float)sphere[2].r / 4.0f);

	Matrix world;
	g_Renderer.getBoneMatrix(Lara.itemNumber, LM_HEAD, &world);

	if (ponytail)
	{
		world = Matrix::CreateTranslation(44, -48, -50) * world;
	}
	else if (youngLara)
	{
		world = Matrix::CreateTranslation(-52, -48, -50) * world;
	}
	else
	{
		world = Matrix::CreateTranslation(-4, -4, -48) * world;
	}

	pos.x = world.Translation().x; 
	pos.y = world.Translation().y; 
	pos.z = world.Translation().z;

	int* bone = &g_Level.Bones[Objects[ID_LARA_HAIR].boneIndex];

	if (Hairs[ponytail][0].initialized)
	{
		Hairs[ponytail][0].initialized = false;
		Hairs[ponytail][0].pos.xPos = pos.x;
		Hairs[ponytail][0].pos.yPos = pos.y;
		Hairs[ponytail][0].pos.zPos = pos.z;

		for (int i = 0; i < HAIR_SEGMENTS; i++, bone += 4)
		{
			world = Matrix::CreateTranslation(Hairs[ponytail][i].pos.xPos, Hairs[ponytail][i].pos.yPos, Hairs[ponytail][i].pos.zPos);		
			world = Matrix::CreateFromYawPitchRoll(TO_RAD(Hairs[ponytail][i].pos.yRot), TO_RAD(Hairs[ponytail][i].pos.xRot), 0) * world;			
			world = Matrix::CreateTranslation(*(bone + 1), *(bone + 2), *(bone + 3)) * world;

			Hairs[ponytail][i + 1].initialized = false;
			Hairs[ponytail][i + 1].pos.xPos = world.Translation().x;
			Hairs[ponytail][i + 1].pos.yPos = world.Translation().y;
			Hairs[ponytail][i + 1].pos.zPos = world.Translation().z;
		}
	}
	else
	{
		Hairs[ponytail][0].pos.xPos = pos.x;
		Hairs[ponytail][0].pos.yPos = pos.y;
		Hairs[ponytail][0].pos.zPos = pos.z;

		short roomNumber = LaraItem->roomNumber;
		int wh;

		if (cutscene)
		{
			wh = NO_HEIGHT;
		}
		else
		{
			int x = LaraItem->pos.xPos + (frame->boundingBox.X1 + frame->boundingBox.X2) / 2;
			int y = LaraItem->pos.yPos + (frame->boundingBox.Y1 + frame->boundingBox.Y2) / 2;
			int z = LaraItem->pos.zPos + (frame->boundingBox.Z1 + frame->boundingBox.Z2) / 2;
			wh = GetWaterHeight(x, y, z, roomNumber);
		}

		for (int i = 1; i < HAIR_SEGMENTS + 1; i++, bone += 4)
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
			{
				height = 32767;
			}

			Hairs[ponytail][i].pos.xPos += Hairs[ponytail][i].hvel.x * 3 / 4;
			Hairs[ponytail][i].pos.yPos += Hairs[ponytail][i].hvel.y * 3 / 4;
			Hairs[ponytail][i].pos.zPos += Hairs[ponytail][i].hvel.z * 3 / 4;

			// TR3 UPV uses a hack which forces Lara water status to dry. 
			// Therefore, we can't directly use water status value to determine hair mode.
			bool dryMode = (Lara.waterStatus == LW_ABOVE_WATER) && (Lara.Vehicle == -1 || g_Level.Items[Lara.Vehicle].objectNumber != ID_UPV);

			if (dryMode)
			{
				if (g_Level.Rooms[roomNumber].flags & ENV_FLAG_WIND)
				{
					Hairs[ponytail][i].pos.xPos += Weather.Wind().x * 2.0f;
					Hairs[ponytail][i].pos.zPos += Weather.Wind().z * 2.0f;
				}

				Hairs[ponytail][i].pos.yPos += 10;

				if (wh != NO_HEIGHT && Hairs[ponytail][i].pos.yPos > wh)
				{
					Hairs[ponytail][i].pos.yPos = wh;
				}
				else if (Hairs[ponytail][i].pos.yPos > height)
				{
					Hairs[ponytail][i].pos.xPos = Hairs[ponytail][0].hvel.x;
					Hairs[ponytail][i].pos.zPos = Hairs[ponytail][0].hvel.z;
				}
			}
			else
			{
				if (Hairs[ponytail][i].pos.yPos < wh)
					Hairs[ponytail][i].pos.yPos = wh;
				else if (Hairs[ponytail][i].pos.yPos > height)
					Hairs[ponytail][i].pos.yPos = height;
			}

			for (int j = 0; j < HAIR_SPHERE; j++)
			{
				int x = Hairs[ponytail][i].pos.xPos - sphere[j].x;
				int y = Hairs[ponytail][i].pos.yPos - sphere[j].y;
				int z = Hairs[ponytail][i].pos.zPos - sphere[j].z;

				int distance = SQUARE(x) + SQUARE(y) + SQUARE(z);

				if (distance < SQUARE(sphere[j].r))
				{
					distance = sqrt(distance);

					if (distance == 0)
						distance = 1;

					Hairs[ponytail][i].pos.xPos = sphere[j].x + x * sphere[j].r / distance;
					Hairs[ponytail][i].pos.yPos = sphere[j].y + y * sphere[j].r / distance;
					Hairs[ponytail][i].pos.zPos = sphere[j].z + z * sphere[j].r / distance;
				}
			}

			int distance = sqrt(SQUARE(Hairs[ponytail][i].pos.zPos - Hairs[ponytail][i - 1].pos.zPos) + SQUARE(Hairs[ponytail][i].pos.xPos - Hairs[ponytail][i - 1].pos.xPos));
			Hairs[ponytail][i - 1].pos.yRot = phd_atan((Hairs[ponytail][i].pos.zPos - Hairs[ponytail][i - 1].pos.zPos), (Hairs[ponytail][i].pos.xPos - Hairs[ponytail][i - 1].pos.xPos));
			Hairs[ponytail][i - 1].pos.xRot = -phd_atan(distance, Hairs[ponytail][i].pos.yPos - Hairs[ponytail][i - 1].pos.yPos);

			world = Matrix::CreateTranslation(Hairs[ponytail][i - 1].pos.xPos, Hairs[ponytail][i - 1].pos.yPos, Hairs[ponytail][i - 1].pos.zPos);
			world = Matrix::CreateFromYawPitchRoll(TO_RAD(Hairs[ponytail][i - 1].pos.yRot), TO_RAD(Hairs[ponytail][i - 1].pos.xRot), 0) * world;

			if (i == HAIR_SEGMENTS)
				world = Matrix::CreateTranslation(*(bone - 3), *(bone - 2), *(bone - 1)) * world;
			else
				world = Matrix::CreateTranslation(*(bone + 1), *(bone + 2), *(bone + 3)) * world;

			Hairs[ponytail][i].pos.xPos = world.Translation().x;
			Hairs[ponytail][i].pos.yPos = world.Translation().y;
			Hairs[ponytail][i].pos.zPos = world.Translation().z;

			Hairs[ponytail][i].hvel.x = Hairs[ponytail][i].pos.xPos - Hairs[ponytail][0].hvel.x;
			Hairs[ponytail][i].hvel.y = Hairs[ponytail][i].pos.yPos - Hairs[ponytail][0].hvel.y;
			Hairs[ponytail][i].hvel.z = Hairs[ponytail][i].pos.zPos - Hairs[ponytail][0].hvel.z;
		}
	}
}
