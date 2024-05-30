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

	static void SpawnSteamShotParticle(const ItemInfo& item, const short steamSize)
	{
		//If camera is far, it won't spawn more particles
		int dx = Camera.pos.x - item.Pose.Position.x;
		int dz = Camera.pos.z - item.Pose.Position.z;

		if (dx < -SMOKE_VISIBILITY_DISTANCE_LIMIT || dx > SMOKE_VISIBILITY_DISTANCE_LIMIT ||
			dz < -SMOKE_VISIBILITY_DISTANCE_LIMIT || dz > SMOKE_VISIBILITY_DISTANCE_LIMIT)
			return;

		//Otherwise, continue
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
		int size = steamSize;

		if (steamSize == 4096)
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

	static void SpawnSmokeEmitterParticle(const ItemInfo& item)
	{
		//If camera is far, it won't spawn more particles
		int dx = Camera.pos.x - item.Pose.Position.x;
		int dz = Camera.pos.z - item.Pose.Position.z;

		if (dx < -SMOKE_VISIBILITY_DISTANCE_LIMIT || dx > SMOKE_VISIBILITY_DISTANCE_LIMIT ||
			dz < -SMOKE_VISIBILITY_DISTANCE_LIMIT || dz > SMOKE_VISIBILITY_DISTANCE_LIMIT)
			return;

		//Otherwise, continue
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
		bool isPipeShot = item.TriggerFlags == 111;
		bool isSteamShotEffect = item.TriggerFlags & 8;

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
		else if (isPipeShot)
		{
			AdjustSmokeEmitterPosition(item, CLICK(2));
		}
		else if (isSteamShotEffect)
		{
			auto& OCB					= item.TriggerFlags;
			auto& steamPauseTimer		= item.ItemFlags[0];
			auto& steamSize			= item.ItemFlags[2];

			steamPauseTimer = OCB / 16;

			AdjustSmokeEmitterPosition(item, CLICK(1));

			if ((signed short)(OCB / 16) <= 0)
			{
				steamSize = 4096;
				OCB |= 4;	//Keep 4 small bits,Ignore the rest
			}
		}
	}

	void SmokeEmitterControl(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (!TriggerActive(&item))
			return;

		//Render Underwater Bubbles
		bool isUnderwater = TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, item.RoomNumber);
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

		//Render Horizontal Steam Shot
		bool isSteamShotEffect = item.TriggerFlags & 8;
		if (isSteamShotEffect)
		{
			bool renderNormalSmoke = false;

			auto& steamPauseTimer			= item.ItemFlags[0];
			auto& steamActiveTimer		= item.ItemFlags[1];
			auto& steamSize				= item.ItemFlags[2];

			if (steamPauseTimer)
			{
				renderNormalSmoke = true;

				steamPauseTimer--;

				if (steamPauseTimer <= 0)
					steamActiveTimer = Random::GenerateInt(30,94);
								
				if (steamSize)
					steamSize -= 256;
			}
			else if (steamSize < 4096)
				steamSize += 256;

			if (steamSize)
			{
				SpawnSteamShotParticle(item, steamSize);

				if (steamActiveTimer)
					steamActiveTimer--;
				else
					steamPauseTimer = item.TriggerFlags >> 4;
			}

			if (!renderNormalSmoke)
				return;
		}

		//Render Normal Smoke
		bool isWibbleCondition = (!(Wibble & 0x0F) && (item.ObjectNumber != ID_SMOKE_EMITTER || !(Wibble & 0x1F)));
		if (isWibbleCondition)
		{
			SpawnSmokeEmitterParticle(item);
		}
	}
}