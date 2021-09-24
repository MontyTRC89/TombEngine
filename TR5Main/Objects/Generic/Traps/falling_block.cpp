#include "framework.h"
#include "effects/debris.h"
#include "collide.h"
#include "camera.h"
#include "animation.h"
#include "level.h"
#include "room.h"
#include "Sound/sound.h"
#include "Specific/prng.h"

using namespace TEN::Math::Random;

constexpr auto FALLINGBLOCK_INITIAL_SPEED = 10;
constexpr auto FALLINGBLOCK_FALL_SPEED = 4;
constexpr auto FALLINGBLOCK_FALL_ROTATION_SPEED = 1;
constexpr auto FALLINGBLOCK_DELAY = 52;
constexpr auto FALLINGBLOCK_WIBBLE = 3;
constexpr auto FALLINGBLOCK_CRUMBLE_DELAY = 100;

void InitialiseFallingBlock(short itemNumber)
{
	auto item = &g_Level.Items[itemNumber];

	g_Level.Items[itemNumber].meshBits = 1;
	TEN::Floordata::AddBridge(itemNumber);

	// Set mutators to 0 by default
	for (int i = 0; i < item->mutator.size(); i++)
		item->mutator[i].Rotation = Vector3::Zero;
}

void FallingBlockCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	if (!item->itemFlags[0] && !item->triggerFlags && item->pos.yPos == l->pos.yPos)
	{
		if (!((item->pos.xPos ^ l->pos.xPos) & 0xFFFFFC00) && !((l->pos.zPos ^ item->pos.zPos) & 0xFFFFFC00))
		{
			SoundEffect(SFX_TR4_ROCK_FALL_CRUMBLE, &item->pos, 0);
			AddActiveItem(itemNum);

			item->itemFlags[0] = 0;
			item->status = ITEM_ACTIVE;
			item->flags |= 0x3E00;
		}
	}
}

void FallingBlockControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->triggerFlags)
	{
		item->triggerFlags--;
	}
	else
	{
		if (item->itemFlags[0])
		{
			if (item->itemFlags[0] < FALLINGBLOCK_DELAY)
			{
				// Subtly shake all meshes separately
				for (int i = 0; i < item->mutator.size(); i++)
				{
					item->mutator[i].Rotation.x = RADIAN * GenerateFloat(-FALLINGBLOCK_WIBBLE, FALLINGBLOCK_WIBBLE);
					item->mutator[i].Rotation.y = RADIAN * GenerateFloat(-FALLINGBLOCK_WIBBLE, FALLINGBLOCK_WIBBLE);
					item->mutator[i].Rotation.z = RADIAN * GenerateFloat(-FALLINGBLOCK_WIBBLE, FALLINGBLOCK_WIBBLE);
				}
			}
			else
			{
				// Make rotational falling movement with some random seed
				for (int i = 0; i < item->mutator.size(); i++)
				{
					auto rotSpeed = i % 2 ? FALLINGBLOCK_FALL_ROTATION_SPEED : -FALLINGBLOCK_FALL_ROTATION_SPEED;
					rotSpeed += i % 3 ? rotSpeed / 2 : rotSpeed;
					item->mutator[i].Rotation.x += RADIAN * rotSpeed + (RADIAN * GenerateFloat(-1, 1));
					item->mutator[i].Rotation.y += RADIAN * rotSpeed + (RADIAN * GenerateFloat(-1, 1));
					item->mutator[i].Rotation.z += RADIAN * rotSpeed + (RADIAN * GenerateFloat(-1, 1));
				}

				if (item->itemFlags[0] == FALLINGBLOCK_DELAY)
					item->itemFlags[1] = FALLINGBLOCK_INITIAL_SPEED;

				if (item->itemFlags[1] > 0)
				{
					item->itemFlags[1] += FALLINGBLOCK_FALL_SPEED;
					item->pos.yPos += item->itemFlags[1];
				}
			}

			item->itemFlags[0]++;

			if (GetDistanceToFloor(itemNumber) >= 0)
			{
				// If crumbled before actual delay (e.g. too low position), force delay to be correct
				if (item->itemFlags[0] < FALLINGBLOCK_DELAY)
					item->itemFlags[0] = FALLINGBLOCK_DELAY;

				// Convert object to shatter item
				MESH* meshpp = &g_Level.Meshes[Objects[item->objectNumber].meshIndex];
				ShatterItem.yRot = item->pos.yRot;
				ShatterItem.meshp = meshpp;
				ShatterItem.sphere.x = item->pos.xPos;
				ShatterItem.sphere.y = item->pos.yPos - STEP_SIZE; // So debris won't spawn below floor
				ShatterItem.sphere.z = item->pos.zPos;
				ShatterItem.bit = 0;
				ShatterObject(&ShatterItem, nullptr, 0, item->roomNumber, false);

				SoundEffect(SFX_TR4_ROCK_FALL_LAND, &item->pos, 0);
				KillItem(itemNumber);
			}
		}
		else
		{
			item->meshBits = -2;
			item->itemFlags[0]++;
			SoundEffect(SFX_TR4_ROCK_FALL_CRUMBLE, &item->pos, 0);
		}
	}
}

std::optional<int> FallingBlockFloor(short itemNumber, int x, int y, int z)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	if (!item->meshBits || item->itemFlags[0] >= FALLINGBLOCK_DELAY)
		return std::nullopt;

	int height = item->pos.yPos + GetBoundsAccurate(item)->Y1;
	return std::optional{ height };
}

std::optional<int> FallingBlockCeiling(short itemNumber, int x, int y, int z)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (!item->meshBits || item->itemFlags[0] >= FALLINGBLOCK_DELAY)
		return std::nullopt;

	int height = item->pos.yPos + GetBoundsAccurate(item)->Y2;
	return std::optional{ height };
}

int FallingBlockFloorBorder(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	return item->pos.yPos + GetBoundsAccurate(item)->Y1;
}

int FallingBlockCeilingBorder(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	return item->pos.yPos + GetBoundsAccurate(item)->Y2;
}
