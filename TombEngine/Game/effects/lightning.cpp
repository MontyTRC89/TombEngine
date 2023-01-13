#include "framework.h"
#include "Game/effects/lightning.h"

#include "Game/animation.h"
#include "Game/effects/bubble.h"
#include "Game/effects/drip.h"
#include "Game/effects/effects.h"
#include "Game/effects/smoke.h"
#include "Game/effects/tomb4fx.h"
#include "Game/Lara/lara.h"
#include "Game/people.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Entities::Creatures::TR5;
using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

namespace TEN::Effects::Lightning
{
	constexpr auto HELICAL_LASER_SEGMENTS_NUM_MAX = 56;
	
	std::vector<ElectricArc>  Lightning		= {};
	std::vector<HelicalLaser> HelicalLasers = {};

	Vector3i LightningPos[6];
	short	 LightningBuffer[1024];
	
	ElectricArc* TriggerLightning(Vector3i* origin, Vector3i* target, unsigned char amplitude, unsigned char r, unsigned char g, unsigned char b, unsigned char life, char flags, char width, char segments)
	{
		auto arc = ElectricArc();

		arc.pos1 = *origin;
		arc.pos2 = ((*origin * 3) + *target) / 4;
		arc.pos3 = ((*target * 3) + *origin) / 4;
		arc.pos4 = *target;
		arc.flags = flags;

		for (int i = 0; i < 9; i++)
		{
			if (arc.flags & 2 || i < 6)
				arc.interpolation[i] = ((unsigned char)(GetRandomControl() % amplitude) - (unsigned char)(amplitude / 2));
			else
				arc.interpolation[i] = 0;
		}

		arc.r = r;
		arc.g = g;
		arc.b = b;
		arc.life = life;
		arc.segments = segments;
		arc.amplitude = amplitude;
		arc.width = width;

		Lightning.push_back(arc);
		return &Lightning[Lightning.size() - 1];
	}

	void TriggerLightningGlow(int x, int y, int z, byte size, byte r, byte g, byte b)
	{
		auto* spark = GetFreeParticle();

		spark->dG = g;
		spark->sG = g;
		spark->life = 4;
		spark->sLife = 4;
		spark->dR = r;
		spark->sR = r;
		spark->colFadeSpeed = 2;
		spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		spark->on = 1;
		spark->dB = b;
		spark->sB = b;
		spark->fadeToBlack = 0;
		spark->x = x;
		spark->y = y;
		spark->z = z;
		spark->xVel = 0;
		spark->yVel = 0;
		spark->zVel = 0;
		spark->flags = SP_DEF | SP_SCALE;
		spark->scalar = 3;
		spark->maxYvel = 0;
		spark->spriteIndex = Objects[ID_MISC_SPRITES].meshIndex;
		spark->gravity = 0;
		spark->dSize = spark->sSize = spark->size = size + (GetRandomControl() & 3);
	}

	void SpawnHelicalLaser(const Vector3& origin, const Vector3& target)
	 {
		auto laser = HelicalLaser();

		laser.NumSegments = HELICAL_LASER_SEGMENTS_NUM_MAX;
		laser.Origin = origin;
		laser.Target = target;
		laser.Orientation2D = Random::GenerateAngle();
		laser.Color = Vector4(0.0f, 1.0f, 0.375f, 1.0f); // Check start opacity.
		laser.LightPosition = origin;
		laser.Life = 17.0f;
		laser.Scale = 0.0f;
		laser.Length = 0.0f;
		laser.LengthEnd = BLOCK(4);
		laser.FadeIn = 8.0f;
		laser.Rotation = 0.0f;

		laser.r = 0;
		laser.b = 255;
		laser.g = 96;

		HelicalLasers.push_back(laser);

		auto origin2 = Vector3i(origin);
		auto target2 = Vector3i(target);

		TriggerLightning(&origin2, &target2, 1, 0, laser.g, laser.b, 20, LI_THININ | LI_THINOUT, 19, 5);	
		TriggerLightning(&origin2, &target2, 1, 110, 255, 250, 20, LI_THININ | LI_THINOUT, 4, 5);	
		TriggerLightningGlow(
			laser.LightPosition.x, laser.LightPosition.y, laser.LightPosition.z,
			Random::GenerateInt(64, 68) << 24, 0, laser.g / 2, laser.b / 2); // TODO: What's up with red??
	 }

	void UpdateHelicalLasers()
	{
		// No active effects; return early.
		if (HelicalLasers.empty())
			return;

		for (auto& laser : HelicalLasers)
		{
			// Set to despawn.
			if (laser.Life <= 0.0f)
				continue;

			laser.Life -= 1.0f;

			if (laser.Life < 16.0f)
			{
				laser.Rotation -= laser.Rotation / 8;
				laser.Scale += 1.0f;
			}
			else if ((int)round(laser.Life) == 16) // TODO: Cleaner way.
			{
				laser.Rotation = MAX_VISIBILITY_DISTANCE;
				laser.Coil = MAX_VISIBILITY_DISTANCE;
				laser.Length = laser.LengthEnd;
				laser.Scale = 4.0f;
			}
			else
			{
				laser.Coil += (MAX_VISIBILITY_DISTANCE - laser.Coil) / 8;

				if ((laser.LengthEnd - laser.Length) <= (laser.LengthEnd / 4))
				{
					laser.Rotation += (MAX_VISIBILITY_DISTANCE - laser.Rotation) / 8;
					laser.Length = laser.LengthEnd;
				}
				else
				{
					laser.Length += (laser.LengthEnd - laser.Length) / 4;
				}

				if (laser.Scale < 4.0f)
					laser.Scale += 1.0f;
			}

			if (laser.FadeIn < 8.0f)
				laser.FadeIn += 1.0f;

			laser.Orientation2D -= laser.Rotation;
		}

		// Despawn inactive effects.
		HelicalLasers.erase(
			std::remove_if(
				HelicalLasers.begin(), HelicalLasers.end(),
				[](const HelicalLaser& laser) { return (laser.Life <= 0.0f); }), HelicalLasers.end());
	}

	void UpdateLightning()
	{
		// No active effects; return early.
		if (Lightning.empty())
			return;

		for (auto& arc : Lightning)
		{
			// Set to despawn.
			if (arc.life <= 0)
				continue;

			// If/when this behaviour is changed, modify AddLightningArc accordingly.
			arc.life -= 2;
			if (arc.life)
			{
				int* positions = (int*)&arc.pos2;
				for (int j = 0; j < 9; j++)
				{
					*positions += 2 * arc.interpolation[j];
					arc.interpolation[j] = (signed char)(arc.interpolation[j] - (arc.interpolation[j] >> 4));
					positions++;
				}
			}
		}

		// Despawn inactive effects.
		Lightning.erase(
			std::remove_if(
				Lightning.begin(), Lightning.end(),
				[](const ElectricArc& arc) { return (arc.life <= 0); }), Lightning.end());
	}

	void CalcLightningSpline(Vector3i* pos, short* buffer, const ElectricArc& arc)
	{
		buffer[0] = pos->x;
		buffer[1] = pos->y;
		buffer[2] = pos->z;

		buffer += 4;

		if (arc.flags & 1)
		{
			int dp = 65536 / (3 * arc.segments - 1);
			int x = dp;

			if (3 * arc.segments - 2 > 0)
			{
				for (int i = 3 * arc.segments - 2; i > 0; i--)
				{
					short sx = LSpline(x, &pos->x, 6);
					buffer[0] = sx + (GetRandomControl() & 0xF) - 8;
					short sy = LSpline(x, &pos->y, 6);
					buffer[1] = sy + (GetRandomControl() & 0xF) - 8;
					short sz = LSpline(x, &pos->z, 6);
					buffer[2] = sz + (GetRandomControl() & 0xF) - 8;

					x += dp;
					buffer += 4;
				}
			}
		}
		else
		{
			int segments = 3 * arc.segments - 1;

			int dx = (pos[5].x - pos->x) / segments;
			int dy = (pos[5].y - pos->y) / segments;
			int dz = (pos[5].z - pos->z) / segments;

			int x = dx + (GetRandomControl() % (2 * arc.amplitude)) - arc.amplitude + pos->x;
			int y = dy + (GetRandomControl() % (2 * arc.amplitude)) - arc.amplitude + pos->y;
			int z = dz + (GetRandomControl() % (2 * arc.amplitude)) - arc.amplitude + pos->z;

			if (3 * arc.segments - 2 > 0)
			{
				for (int i = 3 * arc.segments - 2; i > 0; i--)
				{
					buffer[0] = x;
					buffer[1] = y;
					buffer[2] = z;

					x += dx + GetRandomControl() % (2 * arc.amplitude) - arc.amplitude;
					y += dy + GetRandomControl() % (2 * arc.amplitude) - arc.amplitude;
					z += dz + GetRandomControl() % (2 * arc.amplitude) - arc.amplitude;

					buffer += 4;
				}
			}
		}

		buffer[0] = pos[5].x;
		buffer[1] = pos[5].y;
		buffer[2] = pos[5].z;
	}

	int LSpline(int x, int* knots, int nk)
	{
		int* k;
		int c1, c2, c3, ret, span;

		x *= nk - 3;
		span = x >> 16;

		if (span >= nk - 3)
			span = nk - 4;

		x -= 65536 * span;
		k = &knots[3 * span];
		c1 = k[3] + (k[3] >> 1) - (k[6] >> 1) - k[6] + (k[9] >> 1) + ((-k[0] - 1) >> 1);
		ret = (long long)c1 * x >> 16;
		c2 = ret + 2 * k[6] - 2 * k[3] - (k[3] >> 1) - (k[9] >> 1) + k[0];
		ret = (long long)c2 * x >> 16;
		c3 = ret + (k[6] >> 1) + ((-k[0] - 1) >> 1);
		ret = (long long)c3 * x >> 16;

		return ret + k[3];
	}
}
