#include "framework.h"
#include "Game/effects/lightning.h"

#include "Game/effects/effects.h"
#include "Game/people.h"
#include "Math/Math.h"
#include "Specific/setup.h"

using namespace TEN::Math;

namespace TEN::Effects::Lightning
{
	constexpr auto HELICAL_LASER_SEGMENTS_NUM_MAX = 56;
	
	std::vector<ElectricArc>  ElectricArcs	= {};
	std::vector<HelicalLaser> HelicalLasers = {};

	std::array<Vector3i, 6>	   ElectricArcKnots	   = {};
	std::array<Vector3i, 1024> ElectricArcBuffer = {};
	
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

		ElectricArcs.push_back(arc);
		return &ElectricArcs[ElectricArcs.size() - 1];
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
		spark->dSize =
		spark->sSize =
		spark->size = size + (GetRandomControl() & 3);
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

	void UpdateElectricArcs()
	{
		// No active effects; return early.
		if (ElectricArcs.empty())
			return;

		for (auto& arc : ElectricArcs)
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
					*positions += arc.interpolation[j] * 2;
					arc.interpolation[j] = (signed char)(arc.interpolation[j] - (arc.interpolation[j] / 16));
					positions++;
				}
			}
		}

		// Despawn inactive effects.
		ElectricArcs.erase(
			std::remove_if(
				ElectricArcs.begin(), ElectricArcs.end(),
				[](const ElectricArc& arc) { return (arc.life <= 0.0f); }), ElectricArcs.end());
	}

	void CalculateElectricArcSpline(std::array<Vector3i, 6>& posArray, std::array<Vector3i, 1024>& bufferArray, const ElectricArc& arc)
	{
		int bufferIndex = 0;

		bufferArray[bufferIndex] = posArray[0];
		bufferIndex++;

		// TODO: Amplitude calculation is wrong? Check.

		// Splined arc.
		if (arc.flags & LI_SPLINE)
		{
			int interpStep = 65536 / ((arc.segments * 3) - 1);
			int alpha = interpStep;

			if (((arc.segments * 3) - 2) > 0)
			{
				for (int i = (arc.segments * 3) - 2; i > 0; i--)
				{
					auto spline = Vector3i(
						ElectricArcSpline(alpha, &posArray[0].x, posArray.size()),
						ElectricArcSpline(alpha, &posArray[0].y, posArray.size()),
						ElectricArcSpline(alpha, &posArray[0].z, posArray.size()));

					auto sphere = BoundingSphere(Vector3::Zero, 8.0f);
					auto offset = Random::GeneratePointInSphere(sphere);

					bufferArray[bufferIndex] = spline + offset;
					alpha += interpStep;
					bufferIndex++;
				}
			}
		}
		// Straight arc.
		else
		{
			int numSegments = (arc.segments * 3) - 1;

			auto deltaPos = (posArray[posArray.size() - 1] - posArray[0]) / numSegments;
			auto pos = Vector3i(
				deltaPos.x + (GetRandomControl() % (arc.amplitude * 2)) - arc.amplitude + posArray[0].x,
				deltaPos.y + (GetRandomControl() % (arc.amplitude * 2)) - arc.amplitude + posArray[0].y,
				deltaPos.z + (GetRandomControl() % (arc.amplitude * 2)) - arc.amplitude + posArray[0].z);

			if (((arc.segments * 3) - 2) > 0)
			{
				for (int i = (arc.segments * 3) - 2; i > 0; i--)
				{
					bufferArray[bufferIndex] = pos;
					bufferIndex++;

					pos.x += deltaPos.x + GetRandomControl() % (arc.amplitude * 2) - arc.amplitude;
					pos.y += deltaPos.y + GetRandomControl() % (arc.amplitude * 2) - arc.amplitude;
					pos.z += deltaPos.z + GetRandomControl() % (arc.amplitude * 2) - arc.amplitude;
				}
			}
		}

		bufferArray[bufferIndex] = posArray[5];
	}

	// 4-point Catmull-Rom spline interpolation.
	// NOTE: Alpha is in the range [0, 65536] rather than [0, 1].
	// BIG TODO: Make a family of curve classes with Bezier, BSpline, Catmull-Rom.
	int ElectricArcSpline(int alpha, int* knots, int numKnots)
	{
		alpha *= numKnots - 3;
		int span = alpha >> 16;

		if (span >= (numKnots - 3))
			span = numKnots - 4;

		alpha -= 65536 * span;

		int* k = &knots[3 * span];

		int c1 = k[3] + (k[3] / 2) - (k[6] / 2) - k[6] + (k[9] / 2) + ((-k[0] - 1) / 2);
		int ret = (long long)c1 * alpha >> 16;
		int c2 = ret + 2 * k[6] - 2 * k[3] - (k[3] / 2) - (k[9] / 2) + k[0];
		ret = (long long)c2 * alpha >> 16;
		int c3 = ret + (k[6] / 2) + ((-k[0] - 1) / 2);
		ret = (long long)c3 * alpha >> 16;

		return (ret + k[3]);
	}
}
