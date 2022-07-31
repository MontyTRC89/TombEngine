#include "framework.h"
#include "Game/effects/hair.h"

#include "Game/animation.h"
#include "Game/collision/sphere.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/effects/weather.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Renderer/Renderer11.h"
#include "Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/setup.h"
#include "Specific/level.h"

using namespace TEN::Effects::Environment;
using TEN::Renderer::g_Renderer;

HairData Hairs[HAIR_MAX][HAIR_SEGMENTS + 1];

void InitialiseHair()
{
	for (int h = 0; h < HAIR_MAX; h++)
	{
		int* bone = &g_Level.Bones[Objects[ID_LARA_HAIR].boneIndex];

		Hairs[h][0].Initialized = true;
		Hairs[h][0].Pose.Orientation.SetX(Angle::DegToRad(-90.0f));
		Hairs[h][0].Pose.Orientation.y = 0.0f;

		for (int i = 1; i < HAIR_SEGMENTS + 1; i++, bone += 4)
		{
			Hairs[h][i].Initialized = true;
			Hairs[h][i].Pose.Position = Vector3Int(*(bone + 1), *(bone + 2), *(bone + 3));
			Hairs[h][i].Pose.Orientation = EulerAngles(Angle::DegToRad(-90.0f), 0.0f, 0.0f);
			Hairs[h][i].Velocity = Vector3::Zero;
		}
	}
}

void HairControl(ItemInfo* item, bool young)
{
	HairControl(item, 0, 0);
	if (young)
		HairControl(item, 1, 0);
}

void HairControl(ItemInfo* item, int braid, ANIM_FRAME* framePtr)
{
	auto* lara = GetLaraInfo(item);
	auto* object = &Objects[ID_LARA];

	bool youngLara = g_GameFlow->GetLevel(CurrentLevel)->GetLaraType() == LaraType::Young;

	ANIM_FRAME* frame;
	if (framePtr == NULL)
	{
		if (lara->HitDirection >= 0)
		{
			int spasm;
			switch (lara->HitDirection)
			{
			case NORTH:
				if (lara->Control.IsLow)
					spasm = 294;
				else
					spasm = 125;

				break;

			case SOUTH:
				if (lara->Control.IsLow)
					spasm = 293;
				else
					spasm = 126;

				break;

			case EAST:
				if (lara->Control.IsLow)
					spasm = 295;
				else
					spasm = 127;

				break;

			default:
				if (lara->Control.IsLow)
					spasm = 296;
				else
					spasm = 128;

				break;
			}

			frame = &g_Level.Frames[g_Level.Anims[spasm].framePtr + lara->HitFrame];
		}
		else
			frame = GetBestFrame(item);
	}
	else
		frame = framePtr;

	SPHERE sphere[HAIR_SPHERE];

	// Get Lara's spheres in absolute coords, for head, torso, hips and upper arms
	auto* mesh = &g_Level.Meshes[lara->MeshPtrs[LM_HIPS]];
	auto pos = Vector3Int((int)mesh->sphere.Center.x, (int)mesh->sphere.Center.y, (int)mesh->sphere.Center.z);
	GetLaraJointPosition(&pos, LM_HIPS);

	sphere[0].x = pos.x;
	sphere[0].y = pos.y;
	sphere[0].z = pos.z;
	sphere[0].r = (int)mesh->sphere.Radius;

	mesh = &g_Level.Meshes[lara->MeshPtrs[LM_TORSO]];
	pos = Vector3Int((int)mesh->sphere.Center.x - 10, (int)mesh->sphere.Center.y, (int)mesh->sphere.Center.z + 25); // Repositioning sphere - from tomb5 
	GetLaraJointPosition(&pos, LM_TORSO);

	sphere[1].x = pos.x;
	sphere[1].y = pos.y;
	sphere[1].z = pos.z;
	sphere[1].r = (int)mesh->sphere.Radius;
	if (youngLara)
		sphere[1].r = sphere[1].r - ((sphere[1].r >> 2) + (sphere[1].r >> 3));

	mesh = &g_Level.Meshes[lara->MeshPtrs[LM_HEAD]];
	pos = Vector3Int((int)mesh->sphere.Center.x - 2, (int)mesh->sphere.Center.y, (int)mesh->sphere.Center.z); // Repositioning sphere - from tomb5 
	GetLaraJointPosition(&pos, LM_HEAD);
	sphere[2].x = pos.x;
	sphere[2].y = pos.y;
	sphere[2].z = pos.z;
	sphere[2].r = (int)mesh->sphere.Radius;

	mesh = &g_Level.Meshes[lara->MeshPtrs[LM_RINARM]];
	pos = Vector3Int((int)mesh->sphere.Center.x, (int)mesh->sphere.Center.y, (int)mesh->sphere.Center.z);
	GetLaraJointPosition(&pos, LM_RINARM);
	sphere[3].x = pos.x;
	sphere[3].y = pos.y;
	sphere[3].z = pos.z;
	sphere[3].r = (int)(4.0f * mesh->sphere.Radius / 3.0f); // Resizing sphere - from tomb5 

	mesh = &g_Level.Meshes[lara->MeshPtrs[LM_LINARM]];
	pos = Vector3Int((int)mesh->sphere.Center.x, (int)mesh->sphere.Center.y, (int)mesh->sphere.Center.z);
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

	if (braid)
		world = Matrix::CreateTranslation(44, -48, -50) * world;
	else if (youngLara)
		world = Matrix::CreateTranslation(-52, -48, -50) * world;
	else
		world = Matrix::CreateTranslation(-4, -4, -48) * world;

	pos = Vector3Int(world.Translation().x, world.Translation().y, world.Translation().z);

	int* bone = &g_Level.Bones[Objects[ID_LARA_HAIR].boneIndex];

	if (Hairs[braid][0].Initialized)
	{
		Hairs[braid][0].Initialized = false;
		Hairs[braid][0].Pose.Position = pos;

		for (int i = 0; i < HAIR_SEGMENTS; i++, bone += 4)
		{
			world = Matrix::CreateTranslation(Hairs[braid][i].Pose.Position.x, Hairs[braid][i].Pose.Position.y, Hairs[braid][i].Pose.Position.z);		
			world = Matrix::CreateFromYawPitchRoll(Hairs[braid][i].Pose.Orientation.y, Hairs[braid][i].Pose.Orientation.x, 0) * world;			
			world = Matrix::CreateTranslation(*(bone + 1), *(bone + 2), *(bone + 3)) * world;

			Hairs[braid][i + 1].Initialized = false;
			Hairs[braid][i + 1].Pose.Position = Vector3Int(world.Translation().x, world.Translation().y, world.Translation().z);
		}
	}
	else
	{
		Hairs[braid][0].Pose.Position = pos;

		short roomNumber = item->RoomNumber;
		int x = item->Pose.Position.x + (frame->boundingBox.X1 + frame->boundingBox.X2) / 2;
		int y = item->Pose.Position.y + (frame->boundingBox.Y1 + frame->boundingBox.Y2) / 2;
		int z = item->Pose.Position.z + (frame->boundingBox.Z1 + frame->boundingBox.Z2) / 2;
		int waterHeight = GetWaterHeight(x, y, z, roomNumber);

		for (int i = 1; i < HAIR_SEGMENTS + 1; i++, bone += 4)
		{
			Hairs[braid][0].Velocity = Hairs[braid][i].Pose.Position.ToVector3();

			FloorInfo* floor = GetFloor(Hairs[braid][i].Pose.Position.x, Hairs[braid][i].Pose.Position.y, Hairs[braid][i].Pose.Position.z, &roomNumber);
			int height = GetFloorHeight(floor, Hairs[braid][i].Pose.Position.x, Hairs[braid][i].Pose.Position.y, Hairs[braid][i].Pose.Position.z);

			Hairs[braid][i].Pose.Position.x += Hairs[braid][i].Velocity.x * 3 / 4;
			Hairs[braid][i].Pose.Position.y += Hairs[braid][i].Velocity.y * 3 / 4;
			Hairs[braid][i].Pose.Position.z += Hairs[braid][i].Velocity.z * 3 / 4;

			// TR3 UPV uses a hack which forces Lara water status to dry. 
			// Therefore, we can't directly use water status value to determine hair mode.
			bool dryMode = (lara->Control.WaterStatus == WaterStatus::Dry) && (lara->Vehicle == -1 || g_Level.Items[lara->Vehicle].ObjectNumber != ID_UPV);

			if (dryMode)
			{
				if (TestEnvironment(ENV_FLAG_WIND, roomNumber))
				{
					Hairs[braid][i].Pose.Position.x += Weather.Wind().x * 2.0f;
					Hairs[braid][i].Pose.Position.z += Weather.Wind().z * 2.0f;
				}

				Hairs[braid][i].Pose.Position.y += 10;

				if (waterHeight != NO_HEIGHT && Hairs[braid][i].Pose.Position.y > waterHeight)
					Hairs[braid][i].Pose.Position.y = waterHeight;
				else if (Hairs[braid][i].Pose.Position.y > height)
				{
					Hairs[braid][i].Pose.Position.x = Hairs[braid][0].Velocity.x;
					Hairs[braid][i].Pose.Position.z = Hairs[braid][0].Velocity.z;
				}
			}
			else
			{
				if (Hairs[braid][i].Pose.Position.y < waterHeight)
					Hairs[braid][i].Pose.Position.y = waterHeight;
				else if (Hairs[braid][i].Pose.Position.y > height)
					Hairs[braid][i].Pose.Position.y = height;
			}

			for (int j = 0; j < HAIR_SPHERE; j++)
			{
				int x = Hairs[braid][i].Pose.Position.x - sphere[j].x;
				int y = Hairs[braid][i].Pose.Position.y - sphere[j].y;
				int z = Hairs[braid][i].Pose.Position.z - sphere[j].z;

				int distance = pow(x, 2) + pow(y, 2) + pow(z, 2);

				if (distance < pow(sphere[j].r, 2))
				{
					distance = sqrt(distance);

					if (distance == 0)
						distance = 1;

					Hairs[braid][i].Pose.Position.x = sphere[j].x + x * sphere[j].r / distance;
					Hairs[braid][i].Pose.Position.y = sphere[j].y + y * sphere[j].r / distance;
					Hairs[braid][i].Pose.Position.z = sphere[j].z + z * sphere[j].r / distance;
				}
			}

			int distance = sqrt(pow(Hairs[braid][i].Pose.Position.z - Hairs[braid][i - 1].Pose.Position.z, 2) + pow(Hairs[braid][i].Pose.Position.x - Hairs[braid][i - 1].Pose.Position.x, 2));
			Hairs[braid][i - 1].Pose.Orientation.SetY(atan2((Hairs[braid][i].Pose.Position.z - Hairs[braid][i - 1].Pose.Position.z), (Hairs[braid][i].Pose.Position.x - Hairs[braid][i - 1].Pose.Position.x)));
			Hairs[braid][i - 1].Pose.Orientation.SetX(-atan2(distance, Hairs[braid][i].Pose.Position.y - Hairs[braid][i - 1].Pose.Position.y));

			world = Matrix::CreateTranslation(Hairs[braid][i - 1].Pose.Position.x, Hairs[braid][i - 1].Pose.Position.y, Hairs[braid][i - 1].Pose.Position.z);
			world = Matrix::CreateFromYawPitchRoll(Hairs[braid][i - 1].Pose.Orientation.y, Hairs[braid][i - 1].Pose.Orientation.x, 0) * world;

			if (i == HAIR_SEGMENTS)
				world = Matrix::CreateTranslation(*(bone - 3), *(bone - 2), *(bone - 1)) * world;
			else
				world = Matrix::CreateTranslation(*(bone + 1), *(bone + 2), *(bone + 3)) * world;	

			Hairs[braid][i].Pose.Position = Vector3Int(world.Translation().x, world.Translation().y, world.Translation().z);
			Hairs[braid][i].Velocity = Hairs[braid][i].Pose.Position.ToVector3() - Hairs[braid][0].Velocity;
		}
	}
}
