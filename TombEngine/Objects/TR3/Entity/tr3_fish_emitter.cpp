#include "framework.h"
#include "Objects/TR3/Entity/tr3_fish_emitter.h"

#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/control/flipeffect.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Setup.h"
#include "Specific/level.h"
#include "Math/Math.h"
#include "Renderer/Renderer.h"

using namespace TEN::Math;
using namespace TEN::Renderer;

namespace TEN::Entities::Creatures::TR3
{
	FishData FishSwarm[NUM_FISHES];

	constexpr auto FISH_LARA_DAMAGE = 3;
	constexpr auto FISH_ENTITY_DAMAGE = 1;

	int NextFish;

	void InitializeFishSwarm(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (!item->Pose.Orientation.y)
			item->Pose.Orientation.z += CLICK(2);
		else if (item->Pose.Orientation.y == ANGLE(90.0f))
			item->Pose.Orientation.x += CLICK(2);
		else if (item->Pose.Orientation.y == -ANGLE(180.0f))
			item->Pose.Orientation.z -= CLICK(2);
		else if (item->Pose.Orientation.y == -ANGLE(90.0f))
			item->Pose.Orientation.x -= CLICK(2);

		item->TriggerFlags = 24;
	}



	void FishSwarmControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		//if (!CreatureActive(itemNumber))
			//return;

		if (item->TriggerFlags)
		{
			SpawnFishSwarm(item);
			item->TriggerFlags--;
		}
	
	}

	void SpawnFishSwarm(ItemInfo* item)
	{
		Vector3i origin, target;
		short fishNumber = GetFreeFish();
		EulerAngles orient;
		if (fishNumber != NO_ITEM)
		{
			auto* fish = &FishSwarm[fishNumber];

			fish->on = true;
			fish->Pose.Position = item->Pose.Position;
			fish->Pose.Orientation.x = (GetRandomControl() & 0x3FF) - 512;
			fish->Pose.Orientation.y = (GetRandomControl() & 0x7FF) + item->Pose.Orientation.y + -ANGLE(180.0f) - 1024;
			fish->roomNumber = item->RoomNumber;
			fish->randomRotation = (GetRandomControl() & 0x1F) + 0x10;
			fish->Velocity = (GetRandomControl() & 0x1F) + 16;
			fish->counter = 20 * ((GetRandomControl() & 0x7) + 0xF);
		}
	}

	void ClearFishSwarm()
	{
		if (Objects[ID_FISH_EMITTER].loaded)
		{
			ZeroMemory(FishSwarm, NUM_FISHES * sizeof(FishData));
			NextFish = 0;
			FlipEffect = -1;
		}
	}

	short GetFreeFish()
	{
		for (int i = 0; i < NUM_FISHES; i++)
		{
			auto* fish = &FishSwarm[i];
			if (!fish->on)
				return i;
		}

		return NO_ITEM;
	}

	void UpdateFishSwarm()
	{
		int minDistance = MAXINT;
		int minIndex = -1;

		for (int i = 0; i < NUM_FISHES; i++)
		{
			auto* fish = &FishSwarm[i];

			if (!fish->on)
				continue;


			if (!(GetRandomControl() & 7))
			{
				fish->YTarget = (GetRandomControl() & 0x1F) + 1;
				fish->XTarget = (GetRandomControl() & 0x7F) - 64;
				fish->ZTarget = (GetRandomControl() & 0x7F) - 64;
			}

			auto angles = Geometry::GetOrientToPoint(
				fish->Pose.Position.ToVector3(),
				Vector3(
					LaraItem->Pose.Position.x + fish->XTarget * 8,
					LaraItem->Pose.Position.y + fish->YTarget,
					LaraItem->Pose.Position.z + fish->ZTarget * 8
				));

			int x = LaraItem->Pose.Position.x - fish->Pose.Position.x;
			int z = LaraItem->Pose.Position.z - fish->Pose.Position.z;
			int distance = pow(x, 2) + pow(z, 2);
			if (distance < minDistance)
			{
				minDistance = distance;
				minIndex = i;
			}

			distance = sqrt(distance) / 18;
			if (distance < 48)
				distance = 48;
			else if (distance > 168)
				distance = 168;

			if (fish->Velocity < distance)
				fish->Velocity++;
			else if (fish->Velocity > distance)
				fish->Velocity--;

			//if (fish->Counter > 90)
			//{
				short Velocity = fish->Velocity * 128;

				short xAngle = abs(angles.x - fish->Pose.Orientation.x) / 2;
				short yAngle = abs(angles.y - fish->Pose.Orientation.y) / 2;

				if (xAngle < -Velocity)
					xAngle = -Velocity;
				else if (xAngle > Velocity)
					xAngle = Velocity;

				if (yAngle < -Velocity)
					yAngle = -Velocity;
				else if (yAngle > Velocity)
					yAngle = Velocity;

				fish->Pose.Orientation.y += yAngle;
				fish->Pose.Orientation.x += xAngle;
			//}

			int sp = fish->Velocity * phd_cos(fish->Pose.Orientation.x);

			fish->Pose.Position.x += sp * phd_sin(fish->Pose.Orientation.y);
			fish->Pose.Position.y += fish->Velocity * phd_sin(-fish->Pose.Orientation.x);
			fish->Pose.Position.z += sp * phd_cos(fish->Pose.Orientation.y);

				if (ItemNearTarget(fish->Pose.Position, LaraItem, CLICK(1) / 2))
				{
					TriggerBlood(fish->Pose.Position.x, fish->Pose.Position.y, fish->Pose.Position.z, 2 * GetRandomControl(), 2);
					DoDamage(LaraItem, FISH_LARA_DAMAGE);
				}

				g_Renderer.AddDebugSphere(Vector3(fish->Pose.Position.x, fish->Pose.Position.y, fish->Pose.Position.z), 46, Vector4(1, 0, 0, 1), RendererDebugPage::None);

				Matrix translation = Matrix::CreateTranslation(fish->Pose.Position.x, fish->Pose.Position.y, fish->Pose.Position.z);
				Matrix rotation = fish->Pose.Orientation.ToRotationMatrix();
				fish->Transform = rotation * translation;
			
		}
	}
}
