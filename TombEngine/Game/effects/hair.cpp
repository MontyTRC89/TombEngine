#include "framework.h"
#include "Game/effects/hair.h"

#include "Game/animation.h"
#include "Game/collision/sphere.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/effects/weather.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Renderer/Renderer11.h"
#include "Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/setup.h"
#include "Specific/level.h"

using namespace TEN::Effects::Environment;
using TEN::Renderer::g_Renderer;

HAIR_STRUCT Hairs[HAIR_MAX][HAIR_SEGMENTS + 1];

void InitialiseHair()
{
	for (int h = 0; h < HAIR_MAX; h++)
	{
		int* bone = &g_Level.Bones[Objects[ID_LARA_HAIR].boneIndex];

		Hairs[h][0].initialised = true;
		Hairs[h][0].pos.Orientation.y = 0;
		Hairs[h][0].pos.Orientation.x = -0x4000;

		for (int i = 1; i < HAIR_SEGMENTS + 1; i++, bone += 4)
		{
			Hairs[h][i].initialised = true;
			Hairs[h][i].pos.Position.x = *(bone + 1);
			Hairs[h][i].pos.Position.y = *(bone + 2);
			Hairs[h][i].pos.Position.z = *(bone + 3);
			Hairs[h][i].pos.Orientation.x = -0x4000;
			Hairs[h][i].pos.Orientation.y = Hairs[h][i].pos.Orientation.z = 0;

			Hairs[h][i].hvel.x = Hairs[h][i].hvel.y = Hairs[h][i].hvel.z = 0;
		}
	}
}

void HairControl(ItemInfo* item, bool young)
{
	HairControl(item, 0, 0);
	if (young)
		HairControl(item, 1, 0);
}

void HairControl(ItemInfo* item, int ponytail, ANIM_FRAME* framePtr)
{
	SPHERE sphere[HAIR_SPHERE];
	ObjectInfo* object = &Objects[ID_LARA];
	ANIM_FRAME* frame;
	int spaz;
	bool youngLara = g_GameFlow->GetLevel(CurrentLevel)->GetLaraType() == LaraType::Young;

	LaraInfo*& lara = item->Data;

	if (framePtr == NULL)
	{
		if (lara->HitDirection >= 0)
		{
			switch (lara->HitDirection)
			{
			case NORTH:
				if (lara->Control.IsLow)
					spaz = 294;
				else
					spaz = 125;
				break;

			case SOUTH:
				if (lara->Control.IsLow)
					spaz = 293;
				else
					spaz = 126;
				break;

			case EAST:
				if (lara->Control.IsLow)
					spaz = 295;
				else
					spaz = 127;
				break;

			default:
				if (lara->Control.IsLow)
					spaz = 296;
				else
					spaz = 128;
				break;
			}

			frame = &g_Level.Frames[g_Level.Anims[spaz].framePtr + lara->HitFrame];
		}
		else
			frame = GetBestFrame(item);
	}
	else
	{
		frame = framePtr;
	}

	// Get Lara's spheres in absolute coords, for head, torso, hips and upper arms
	MESH* mesh = &g_Level.Meshes[lara->MeshPtrs[LM_HIPS]];
	Vector3Int pos = { (int)mesh->sphere.Center.x, (int)mesh->sphere.Center.y, (int)mesh->sphere.Center.z };
	GetLaraJointPosition(&pos, LM_HIPS);
	sphere[0].x = pos.x;
	sphere[0].y = pos.y;
	sphere[0].z = pos.z;
	sphere[0].r = (int)mesh->sphere.Radius;

	mesh = &g_Level.Meshes[lara->MeshPtrs[LM_TORSO]];
	pos = { (int)mesh->sphere.Center.x - 10, (int)mesh->sphere.Center.y, (int)mesh->sphere.Center.z + 25 }; // Repositioning sphere - from tomb5 
	GetLaraJointPosition(&pos, LM_TORSO);
	sphere[1].x = pos.x;
	sphere[1].y = pos.y;
	sphere[1].z = pos.z;
	sphere[1].r = (int)mesh->sphere.Radius;
	if (youngLara)
		sphere[1].r = sphere[1].r - ((sphere[1].r >> 2) + (sphere[1].r >> 3));

	mesh = &g_Level.Meshes[lara->MeshPtrs[LM_HEAD]];
	pos = { (int)mesh->sphere.Center.x - 2, (int)mesh->sphere.Center.y, (int)mesh->sphere.Center.z }; // Repositioning sphere - from tomb5 
	GetLaraJointPosition(&pos, LM_HEAD);
	sphere[2].x = pos.x;
	sphere[2].y = pos.y;
	sphere[2].z = pos.z;
	sphere[2].r = (int)mesh->sphere.Radius;

	mesh = &g_Level.Meshes[lara->MeshPtrs[LM_RINARM]];
	pos = { (int)mesh->sphere.Center.x, (int)mesh->sphere.Center.y, (int)mesh->sphere.Center.z };
	GetLaraJointPosition(&pos, LM_RINARM);
	sphere[3].x = pos.x;
	sphere[3].y = pos.y;
	sphere[3].z = pos.z;
	sphere[3].r = (int)(4.0f * mesh->sphere.Radius / 3.0f); // Resizing sphere - from tomb5 

	mesh = &g_Level.Meshes[lara->MeshPtrs[LM_LINARM]];
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
	g_Renderer.GetBoneMatrix(lara->ItemNumber, LM_HEAD, &world);

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

	if (Hairs[ponytail][0].initialised)
	{
		Hairs[ponytail][0].initialised = false;
		Hairs[ponytail][0].pos.Position.x = pos.x;
		Hairs[ponytail][0].pos.Position.y = pos.y;
		Hairs[ponytail][0].pos.Position.z = pos.z;

		for (int i = 0; i < HAIR_SEGMENTS; i++, bone += 4)
		{
			world = Matrix::CreateTranslation(Hairs[ponytail][i].pos.Position.x, Hairs[ponytail][i].pos.Position.y, Hairs[ponytail][i].pos.Position.z);		
			world = Matrix::CreateFromYawPitchRoll(TO_RAD(Hairs[ponytail][i].pos.Orientation.y), TO_RAD(Hairs[ponytail][i].pos.Orientation.x), 0) * world;			
			world = Matrix::CreateTranslation(*(bone + 1), *(bone + 2), *(bone + 3)) * world;

			Hairs[ponytail][i + 1].initialised = false;
			Hairs[ponytail][i + 1].pos.Position.x = world.Translation().x;
			Hairs[ponytail][i + 1].pos.Position.y = world.Translation().y;
			Hairs[ponytail][i + 1].pos.Position.z = world.Translation().z;
		}
	}
	else
	{
		Hairs[ponytail][0].pos.Position.x = pos.x;
		Hairs[ponytail][0].pos.Position.y = pos.y;
		Hairs[ponytail][0].pos.Position.z = pos.z;

		short roomNumber = item->RoomNumber;
		int x = item->Pose.Position.x + (frame->boundingBox.X1 + frame->boundingBox.X2) / 2;
		int y = item->Pose.Position.y + (frame->boundingBox.Y1 + frame->boundingBox.Y2) / 2;
		int z = item->Pose.Position.z + (frame->boundingBox.Z1 + frame->boundingBox.Z2) / 2;
		int wh = GetWaterHeight(x, y, z, roomNumber);

		for (int i = 1; i < HAIR_SEGMENTS + 1; i++, bone += 4)
		{
			Hairs[ponytail][0].hvel.x = Hairs[ponytail][i].pos.Position.x;
			Hairs[ponytail][0].hvel.y = Hairs[ponytail][i].pos.Position.y;
			Hairs[ponytail][0].hvel.z = Hairs[ponytail][i].pos.Position.z;

			auto floor = GetFloor(Hairs[ponytail][i].pos.Position.x, Hairs[ponytail][i].pos.Position.y, Hairs[ponytail][i].pos.Position.z, &roomNumber);
			int height = GetFloorHeight(floor, Hairs[ponytail][i].pos.Position.x, Hairs[ponytail][i].pos.Position.y, Hairs[ponytail][i].pos.Position.z);

			Hairs[ponytail][i].pos.Position.x += Hairs[ponytail][i].hvel.x * 3 / 4;
			Hairs[ponytail][i].pos.Position.y += Hairs[ponytail][i].hvel.y * 3 / 4;
			Hairs[ponytail][i].pos.Position.z += Hairs[ponytail][i].hvel.z * 3 / 4;

			// TR3 UPV uses a hack which forces Lara water status to dry. 
			// Therefore, we can't directly use water status value to determine hair mode.
			bool dryMode = (lara->Control.WaterStatus == WaterStatus::Dry) && (lara->Vehicle == -1 || g_Level.Items[lara->Vehicle].ObjectNumber != ID_UPV);

			if (dryMode)
			{
				if (g_Level.Rooms[roomNumber].flags & ENV_FLAG_WIND)
				{
					Hairs[ponytail][i].pos.Position.x += Weather.Wind().x * 2.0f;
					Hairs[ponytail][i].pos.Position.z += Weather.Wind().z * 2.0f;
				}

				Hairs[ponytail][i].pos.Position.y += 10;

				if (wh != NO_HEIGHT && Hairs[ponytail][i].pos.Position.y > wh)
				{
					Hairs[ponytail][i].pos.Position.y = wh;
				}
				else if (Hairs[ponytail][i].pos.Position.y > height)
				{
					Hairs[ponytail][i].pos.Position.x = Hairs[ponytail][0].hvel.x;
					Hairs[ponytail][i].pos.Position.z = Hairs[ponytail][0].hvel.z;
				}
			}
			else
			{
				if (Hairs[ponytail][i].pos.Position.y < wh)
					Hairs[ponytail][i].pos.Position.y = wh;
				else if (Hairs[ponytail][i].pos.Position.y > height)
					Hairs[ponytail][i].pos.Position.y = height;
			}

			for (int j = 0; j < HAIR_SPHERE; j++)
			{
				int x = Hairs[ponytail][i].pos.Position.x - sphere[j].x;
				int y = Hairs[ponytail][i].pos.Position.y - sphere[j].y;
				int z = Hairs[ponytail][i].pos.Position.z - sphere[j].z;

				int distance = SQUARE(x) + SQUARE(y) + SQUARE(z);

				if (distance < SQUARE(sphere[j].r))
				{
					distance = sqrt(distance);

					if (distance == 0)
						distance = 1;

					Hairs[ponytail][i].pos.Position.x = sphere[j].x + x * sphere[j].r / distance;
					Hairs[ponytail][i].pos.Position.y = sphere[j].y + y * sphere[j].r / distance;
					Hairs[ponytail][i].pos.Position.z = sphere[j].z + z * sphere[j].r / distance;
				}
			}

			int distance = sqrt(SQUARE(Hairs[ponytail][i].pos.Position.z - Hairs[ponytail][i - 1].pos.Position.z) + SQUARE(Hairs[ponytail][i].pos.Position.x - Hairs[ponytail][i - 1].pos.Position.x));
			Hairs[ponytail][i - 1].pos.Orientation.y = phd_atan((Hairs[ponytail][i].pos.Position.z - Hairs[ponytail][i - 1].pos.Position.z), (Hairs[ponytail][i].pos.Position.x - Hairs[ponytail][i - 1].pos.Position.x));
			Hairs[ponytail][i - 1].pos.Orientation.x = -phd_atan(distance, Hairs[ponytail][i].pos.Position.y - Hairs[ponytail][i - 1].pos.Position.y);

			world = Matrix::CreateTranslation(Hairs[ponytail][i - 1].pos.Position.x, Hairs[ponytail][i - 1].pos.Position.y, Hairs[ponytail][i - 1].pos.Position.z);
			world = Matrix::CreateFromYawPitchRoll(TO_RAD(Hairs[ponytail][i - 1].pos.Orientation.y), TO_RAD(Hairs[ponytail][i - 1].pos.Orientation.x), 0) * world;

			if (i == HAIR_SEGMENTS)
				world = Matrix::CreateTranslation(*(bone - 3), *(bone - 2), *(bone - 1)) * world;
			else
				world = Matrix::CreateTranslation(*(bone + 1), *(bone + 2), *(bone + 3)) * world;

			Hairs[ponytail][i].pos.Position.x = world.Translation().x;
			Hairs[ponytail][i].pos.Position.y = world.Translation().y;
			Hairs[ponytail][i].pos.Position.z = world.Translation().z;

			Hairs[ponytail][i].hvel.x = Hairs[ponytail][i].pos.Position.x - Hairs[ponytail][0].hvel.x;
			Hairs[ponytail][i].hvel.y = Hairs[ponytail][i].pos.Position.y - Hairs[ponytail][0].hvel.y;
			Hairs[ponytail][i].hvel.z = Hairs[ponytail][i].pos.Position.z - Hairs[ponytail][0].hvel.z;
		}
	}
}
