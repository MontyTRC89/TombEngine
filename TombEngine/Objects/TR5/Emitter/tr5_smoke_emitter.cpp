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
// OCB 0:		Default smoke behaviour.
// OCB <0:		Disables the harm from the steam shot by setting the NoDamage flag in ItemFlags[3].
// OCB != 0:	Enables horizontal steam shot effect.
// OCB >> 4:	Sets the number of frames it pauses between steam shots, is calculated like x = 16 * number of frames.
// 
// item.ItemFlags[0]: Timer for the pause between steam shots
// item.ItemFlags[1]: Timer for the active steam shots
// item.ItemFlags[2]: Acceleration of the steam shot particles
// item.ItemFlags[3]: Smoke Emiter Flags
// 
// In Underwater rooms
// OCB 0: Intermitent bubbles emission.
// OCB 1: Continuous bubbles emission.
// OCB +2: Uses big bubbles.
// 
// item.ItemFlags[0]: Count the number of bubles it has to spawn.
// item.ItemFlags[1]: Flag used to spawn a serie of bubbles with no delay.
// item.ItemFlags[2]: Radius of the bubbles spawn horizontal plane. (by default is 32).

namespace TEN::Effects::SmokeEmitter
{
	enum SmokeEmitterFlags
	{
		NoDamage = (1 << 0),
	};

	constexpr auto SMOKE_VISIBILITY_DISTANCE_LIMIT = BLOCK(16);

	static void SpawnSteamShotParticle(const ItemInfo& item, const short currentAcceleration)
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

		if (item.ObjectNumber == ID_SMOKE_EMITTER_BLACK)
		{
			sptr->sR = 96;
			sptr->sG = 96;
			sptr->sB = 96;

			sptr->dR = 96;
			sptr->dG = 96;
			sptr->dB = 96;
		}
		else if (item.ObjectNumber == ID_SMOKE_EMITTER_WHITE)
		{
			sptr->sR = 96;
			sptr->sG = 96;
			sptr->sB = 96;

			sptr->dR = 64;
			sptr->dG = 64;
			sptr->dB = 64;
		}
		else
		{
			unsigned char r = std::clamp(item.Model.Color.x / 2.0f, 0.0f, 1.0f) * UCHAR_MAX;
			unsigned char g = std::clamp(item.Model.Color.y / 2.0f, 0.0f, 1.0f) * UCHAR_MAX;
			unsigned char b = std::clamp(item.Model.Color.z / 2.0f, 0.0f, 1.0f) * UCHAR_MAX;

			sptr->sR = r / 3;
			sptr->sG = g / 3;
			sptr->sB = b / 3;

			sptr->dR = r;
			sptr->dG = g;
			sptr->dB = b;
		}
		
		sptr->fadeToBlack = 6;
		sptr->colFadeSpeed = Random::GenerateInt(6, 9);

		if (item.ObjectNumber == ID_SMOKE_EMITTER_BLACK)
			sptr->blendMode = BlendMode::Subtractive;
		else
			sptr->blendMode = BlendMode::Additive;

		sptr->life = sptr->sLife = Random::GenerateInt(16, 24);

		sptr->x = item.Pose.Position.x + Random::GenerateInt(-32, 32);
		sptr->y = item.Pose.Position.y + Random::GenerateInt(-32, 32);
		sptr->z = item.Pose.Position.z + Random::GenerateInt(-32, 32);

		int acceleration = currentAcceleration;

		if (currentAcceleration == 4096)
			acceleration = Random::GenerateInt(2048, 4095);

		int pitchAngle = item.Pose.Orientation.x;
		int yawAngle = item.Pose.Orientation.y + ANGLE(180);

		Vector3 dir;
		dir.x = phd_cos(pitchAngle) * phd_sin(yawAngle);
		dir.y = phd_sin(pitchAngle);
		dir.z = phd_cos(pitchAngle) * phd_cos(yawAngle);

		dir.Normalize();
		int randomOffset = Random::GenerateInt(-8, 8);
		sptr->xVel = dir.x * acceleration + randomOffset;
		sptr->yVel = dir.y * acceleration + randomOffset;
		sptr->zVel = dir.z * acceleration + randomOffset;

		sptr->friction = 4;
		sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;

		bool ignoreDamage = item.ItemFlags[3] & SmokeEmitterFlags::NoDamage;
		if (!(GlobalCounter & 0x03) && !ignoreDamage)
			sptr->flags |= SP_DAMAGE;

		sptr->rotAng = Random::GenerateInt(0, 4096);

		if (Random::TestProbability(0.5f))
			sptr->rotAdd = Random::GenerateInt(-15, -7);
		else
			sptr->rotAdd = Random::GenerateInt(7,15);

		sptr->scalar = 2;
		sptr->gravity = Random::GenerateInt(-24, -15);
		sptr->maxYvel = Random::GenerateInt(-15, -8);
		
		int particleSize = Random::GenerateInt(128, 160);
		sptr->dSize = float(particleSize);
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

		if (item.ObjectNumber == ID_SMOKE_EMITTER_BLACK)
		{
			sptr->dR = 96;
			sptr->dG = 96;
			sptr->dB = 96;
		}
		else if (item.ObjectNumber == ID_SMOKE_EMITTER_WHITE)
		{
			sptr->dR = 64;
			sptr->dG = 64;
			sptr->dB = 64;
		}
		else
		{
			unsigned char r = std::clamp(item.Model.Color.x / 2.0f, 0.0f, 1.0f) * UCHAR_MAX;
			unsigned char g = std::clamp(item.Model.Color.y / 2.0f, 0.0f, 1.0f) * UCHAR_MAX;
			unsigned char b = std::clamp(item.Model.Color.z / 2.0f, 0.0f, 1.0f) * UCHAR_MAX;

			sptr->dR = r;
			sptr->dG = g;
			sptr->dB = b;
		}

		sptr->fadeToBlack = 16;
		sptr->colFadeSpeed = Random::GenerateInt(8, 11);

		if (item.ObjectNumber == ID_SMOKE_EMITTER_BLACK)
			sptr->blendMode = BlendMode::Subtractive;
		else
			sptr->blendMode = BlendMode::Additive;

		sptr->sLife = sptr->life = Random::GenerateInt(28, 35);

		sptr->x = item.Pose.Position.x + Random::GenerateInt(-32, 32);
		sptr->y = item.Pose.Position.y + Random::GenerateInt(-32, 32);
		sptr->z = item.Pose.Position.z + Random::GenerateInt(-32, 32);

		sptr->xVel = Random::GenerateInt(-128, 128);
		sptr->yVel = Random::GenerateInt(-16, 0);
		sptr->zVel = Random::GenerateInt(-128, 128);

		sptr->friction = 3;
		sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;

		if (TestEnvironment(RoomEnvFlags::ENV_FLAG_OUTSIDE, item.RoomNumber))
			sptr->flags |= SP_WIND;

		sptr->rotAng = Random::GenerateInt(0, 4095);

		if (Random::TestProbability(0.5f))
			sptr->rotAdd = Random::GenerateInt(-15, -7);
		else
			sptr->rotAdd = Random::GenerateInt(7, 15);

		sptr->scalar = 2;
		sptr->gravity = Random::GenerateInt(-24, -15);
		sptr->maxYvel = Random::GenerateInt(-15, -7);

		int size = Random::GenerateInt(128, 160);
		sptr->dSize = float(size);
		sptr->sSize = sptr->size = float(size / 4);

		if (item.ObjectNumber == ID_SMOKE_EMITTER)
		{
			sptr->gravity /= 2;
			sptr->yVel /= 2;
			sptr->maxYvel /= 2;
			sptr->life += 16;
			sptr->sLife += 16;
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
		bool isSteamShotEffect = item.TriggerFlags != 0;

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
		else if (isSteamShotEffect)
		{
			auto& OCB					= item.TriggerFlags;
			auto& steamPauseTimer		= item.ItemFlags[0];
			auto& steamAcceleration	= item.ItemFlags[2];
			auto& steamFlags			= item.ItemFlags[3];

			if (OCB < 0)
			{
				OCB = -OCB;
				steamFlags |= SmokeEmitterFlags::NoDamage;
			}

			steamPauseTimer = OCB / 16;

			if ((signed short)(steamPauseTimer) <= 0)
			{
				steamAcceleration = 4096;
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
			bool effectBigBubbles = item.TriggerFlags & 2;

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
		bool isSteamShotEffect = item.TriggerFlags != 0;
		if (isSteamShotEffect)
		{
			bool renderNormalSmoke = false;

			auto& steamPauseTimer			= item.ItemFlags[0];
			auto& steamActiveTimer		= item.ItemFlags[1];
			auto& steamAcceleration		= item.ItemFlags[2];

			if (steamPauseTimer)
			{
				renderNormalSmoke = true;

				steamPauseTimer--;

				if (steamPauseTimer <= 0)
					steamActiveTimer = Random::GenerateInt(30,94);
								
				if (steamAcceleration)
					steamAcceleration -= 256;
			}
			else if (steamAcceleration < 4096)
				steamAcceleration += 256;

			if (steamAcceleration)
			{
				SpawnSteamShotParticle(item, steamAcceleration);

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