#include "framework.h"
#include "tr5_smoke_emitter.h"

#include "Game/camera.h"
#include "Game/items.h"
#include "Game/effects/Bubble.h"
#include "Game/effects/effects.h"
#include "Game/control/control.h"
#include "Game/control/trigger.h"
#include "Game/collision/collide_room.h"
#include "Specific/level.h"
#include "Math/Math.h"
#include "Objects/objectslist.h"
#include "Renderer/RendererEnums.h"

using namespace TEN::Effects::Bubble;

// NOTES:
// OCB 0: Standard behaviour.
// OCB 1: When underwater, it emits bubles continuously.
// 
// item.ItemFlags[0]: When underwater, it is used to count the number of bubles it has to spawn.
// item.ItemFlags[1]: When underwater, it's a flag used to spawn a serie of bubbles with no delay.

namespace TEN::Effects::SmokeEmitter
{
	constexpr auto SMOKE_VISIBILITY_DISTANCE_LIMIT = BLOCK(16);

	static void AdjustSmokeEmitterPosition(ItemInfo& item, float offset)
	{
		switch (item.Pose.Orientation.y)
		{
		case ANGLE(90.0f):
			item.Pose.Position.x += offset;
			break;
		case ANGLE(-180.0f):
			item.Pose.Position.z -= offset;
			break;
		case ANGLE(-90.0f):
			item.Pose.Position.x -= offset;
			break;
		default:
			item.Pose.Position.z += offset;
			break;
		}
	}

	static void SpawnSmokeEmitterParticle(const ItemInfo& item)
	{
		auto* sptr = GetFreeParticle();
		sptr->on = true;
		sptr->sR = 96;
		sptr->sG = 96;
		sptr->sB = 96;
		sptr->dR = 48;
		sptr->dG = 48;
		sptr->dB = 48;
		sptr->fadeToBlack = 6;
		sptr->colFadeSpeed = (GetRandomControl() & 3) + 6;
		sptr->blendMode = BlendMode::Additive;
		sptr->life = (GetRandomControl() & 7) + 16;
		sptr->sLife = sptr->life;
		sptr->x = (GetRandomControl() & 0x3F) + item.Pose.Position.x - 32;
		sptr->y = (GetRandomControl() & 0x3F) + item.Pose.Position.y - 32;
		sptr->z = (GetRandomControl() & 0x3F) + item.Pose.Position.z - 32;
		int size = item.ItemFlags[2];

		if (item.ItemFlags[2] == 4096)
			size = (GetRandomControl() & 0x7FF) + 2048;

		sptr->xVel = (short)((size * phd_sin(item.Pose.Orientation.y - 32768)) / BLOCK(1));
		sptr->yVel = -16 - (GetRandomControl() & 0xF);
		sptr->zVel = (short)((size * phd_cos(item.Pose.Orientation.y - 32768)) / BLOCK(1));
		sptr->friction = 4;
		sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;

		if (!(GlobalCounter & 0x03))
			sptr->flags |= SP_DAMAGE;

		sptr->rotAng = GetRandomControl() & 0xFFF;

		if (GetRandomControl() & 1)
			sptr->rotAdd = -8 - (GetRandomControl() & 7);
		else
			sptr->rotAdd = (GetRandomControl() & 7) + 8;

		sptr->scalar = 2;
		sptr->gravity = -8 - (GetRandomControl() & 0xF);
		sptr->maxYvel = -8 - (GetRandomControl() & 7);
		size = (GetRandomControl() & 0x1F) + 128;
		sptr->dSize = float(size);
		sptr->sSize = sptr->size = sptr->dSize / 2.0f;
	}

	static void SpawnSmokeEmitterBubble(const ItemInfo& item, int radius, bool bigBubbles)
	{
		int maxRadius = (int)radius;
		int minRadius = (int)(0 - radius);
		auto pos = Vector3(
			item.Pose.Position.x + Random::GenerateInt(minRadius, maxRadius),
			item.Pose.Position.y - Random::GenerateInt(-16, 16),
			item.Pose.Position.z + Random::GenerateInt(minRadius, maxRadius));

		if (bigBubbles)
			SpawnBubble(pos, item.RoomNumber, (int)BubbleFlags::HighAmplitude | (int)BubbleFlags::LargeScale);
		else
			SpawnBubble(pos, item.RoomNumber);
	}

	void InitializeSmokeEmitter(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		bool isUnderwater = TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, item.RoomNumber);
		bool isHorizontalShot = item.TriggerFlags == 111;
		bool isEmitterType = item.TriggerFlags & 8;

		if (isUnderwater)
		{
			bool effectContinuousBubbles = item.TriggerFlags == 1;

			auto& bubbleCount = item.ItemFlags[0];
			auto& bubbleForceSpawnFlag = item.ItemFlags[1];
			auto& bubbleSpawnRadius = item.ItemFlags[2];

			if (effectContinuousBubbles)
			{
				bubbleCount = 20;
				bubbleForceSpawnFlag = 1;
			}

			if (bubbleSpawnRadius == 0)
				bubbleSpawnRadius = 32; //Default value
		}
		else if (isHorizontalShot)
		{
			AdjustSmokeEmitterPosition(item, CLICK(2));
		}
		else if (isEmitterType)
		{
			item.ItemFlags[0] = item.TriggerFlags / 16;

			AdjustSmokeEmitterPosition(item, CLICK(1));

			if ((signed short)(item.TriggerFlags / 16) <= 0)
			{
				item.ItemFlags[2] = 4096;
				item.TriggerFlags |= 4;
			}
		}
	}

	void SmokeEmitterControl(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (!TriggerActive(&item))
			return;

		bool isUnderwater = TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, item.RoomNumber);
		bool isSmokeEmitterType = item.ObjectNumber == ID_SMOKE_EMITTER;
		bool isUnknown = item.TriggerFlags & 8;
		bool isWibbleCondition = (!(Wibble & 0x0F) && (item.ObjectNumber != ID_SMOKE_EMITTER || !(Wibble & 0x1F)));

		if (isUnderwater)
		{
			bool effectContinuousBubbles = item.TriggerFlags == 1;
			bool effectBigBubbles = item.TriggerFlags == 2;

			auto& bubbleCount = item.ItemFlags[0];
			auto& bubbleForceSpawnFlag = item.ItemFlags[1];
			auto& bubbleSpawnRadius = item.ItemFlags[2];

			if (bubbleCount || Random::TestProbability(1 / 30.0f) || effectContinuousBubbles)
			{
				if (Random::TestProbability(1 / 3.0f) || bubbleForceSpawnFlag)
				{
					SpawnSmokeEmitterBubble(item, bubbleSpawnRadius, effectBigBubbles);

					if (bubbleCount)
					{
						bubbleCount--;

						if (!bubbleCount)
							bubbleForceSpawnFlag = 0;
					}
				}
			}
			else if (Random::TestProbability(1 / 30.0f))
			{
				bubbleCount = Random::GenerateInt(4, 7);
			}

			return;
		}

		if (isSmokeEmitterType && isUnknown)
		{
			bool normal = false;

			auto& generalCounter = item.ItemFlags[0];
			auto& stateID = item.ItemFlags[1];
			auto& secondaryCounter = item.ItemFlags[2];

			if (generalCounter)
			{
				generalCounter--;

				if (!generalCounter)
					stateID = (GetRandomControl() & 0x3F) + 30;

				normal = true;

				if (secondaryCounter)
					secondaryCounter -= 256;
			}
			else if (secondaryCounter < 4096)
				secondaryCounter += 256;

			if (secondaryCounter)
			{
				//If camera is far, it won't spawn more particles
				int dx = Camera.pos.x - item.Pose.Position.x;
				int dz = Camera.pos.z - item.Pose.Position.z;

				if (dx < -SMOKE_VISIBILITY_DISTANCE_LIMIT || dx > SMOKE_VISIBILITY_DISTANCE_LIMIT  || 
					dz < -SMOKE_VISIBILITY_DISTANCE_LIMIT || dz > SMOKE_VISIBILITY_DISTANCE_LIMIT)
					return;

				//Otherwise, continue
				SpawnSmokeEmitterParticle(item);

				if (stateID)
					stateID--;
				else
					generalCounter = item.TriggerFlags >> 4;
			}

			if (!normal)
				return;
		}

		if (isWibbleCondition)
		{
			int dx = Camera.pos.x - item.Pose.Position.x;
			int dz = Camera.pos.z - item.Pose.Position.z;

			if (dx < -BLOCK(16) || dx > BLOCK(16) || dz < -BLOCK(16) || dz > BLOCK(16))
				return;

			auto* sptr = GetFreeParticle();
			sptr->on = 1;
			sptr->sR = 0;
			sptr->sG = 0;
			sptr->sB = 0;
			sptr->dR = 64;
			sptr->dG = 64;
			sptr->dB = 64;

			if (item.ObjectNumber == ID_SMOKE_EMITTER_BLACK)
			{
				sptr->dR = 96;
				sptr->dG = 96;
				sptr->dB = 96;
			}

			sptr->fadeToBlack = 16;
			sptr->colFadeSpeed = (GetRandomControl() & 3) + 8;
			sptr->sLife = sptr->life = (GetRandomControl() & 7) + 28;

			if (item.ObjectNumber == ID_SMOKE_EMITTER_BLACK)
				sptr->blendMode = BlendMode::Subtractive;
			else
				sptr->blendMode = BlendMode::Additive;

			sptr->x = (GetRandomControl() & 0x3F) + item.Pose.Position.x - 32;
			sptr->y = (GetRandomControl() & 0x3F) + item.Pose.Position.y - 32;
			sptr->z = (GetRandomControl() & 0x3F) + item.Pose.Position.z - 32;
			sptr->xVel = (GetRandomControl() & 0xFF) - 128;
			sptr->yVel = -16 - (GetRandomControl() & 0xF);
			sptr->zVel = (GetRandomControl() & 0xFF) - 128;
			sptr->friction = 3;
			sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;

			if (TestEnvironment(RoomEnvFlags::ENV_FLAG_OUTSIDE, item.RoomNumber))
				sptr->flags |= SP_WIND;

			sptr->rotAng = GetRandomControl() & 0xFFF;

			if (GetRandomControl() & 1)
				sptr->rotAdd = -8 - (GetRandomControl() & 7);
			else
				sptr->rotAdd = (GetRandomControl() & 7) + 8;

			sptr->scalar = 2;
			sptr->gravity = -8 - (GetRandomControl() & 0xF);
			sptr->maxYvel = -8 - (GetRandomControl() & 7);
			int size = (GetRandomControl() & 0x1F) + 128;
			sptr->dSize = float(size);
			sptr->sSize = sptr->size = float(size / 4);

			if (item.ObjectNumber == ID_SMOKE_EMITTER)
			{
				sptr->gravity /= 2;
				sptr->yVel /= 2;
				sptr->maxYvel /= 2;
				sptr->life += 16;
				sptr->sLife += 16;
				sptr->dR = 32;
				sptr->dG = 32;
				sptr->dB = 32;
			}
		}
	}
}