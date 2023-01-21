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

	std::array<Vector3, 6>	  ElectricArcKnots	= {};
	std::array<Vector3, 1024> ElectricArcBuffer = {};
	
	// TODO: Pass const Vector3 references to postions and Vector3/Vector4 for color.
	void TriggerLightning(Vector3i* origin, Vector3i* target, float amplitude, byte r, byte g, byte b, float life, int flags, float width, unsigned int numSegments)
	{
		auto arc = ElectricArc();

		auto fltOrigin = origin->ToVector3();
		auto fltTarget = target->ToVector3();

		arc.pos1 = fltOrigin;
		arc.pos2 = ((fltOrigin * 3) + fltTarget) / 4;
		arc.pos3 = ((fltTarget * 3) + fltOrigin) / 4;
		arc.pos4 = fltTarget;
		arc.flags = flags;

		for (int i = 0; i < arc.interpolation.size(); i++)
		{
			if (arc.flags & LI_MOVEEND || i < (arc.interpolation.size() - 1))
			{
				arc.interpolation[i] = Vector3(
					fmod(Random::GenerateInt(), amplitude),
					fmod(Random::GenerateInt(), amplitude),
					fmod(Random::GenerateInt(), amplitude)) -
					Vector3(amplitude/ 2);
			}
			else
			{
				arc.interpolation[i] = Vector3::Zero;
			}
		}

		arc.r = r;
		arc.g = g;
		arc.b = b;
		arc.life = life;
		arc.segments = numSegments;
		arc.amplitude = amplitude;
		arc.width = width;

		ElectricArcs.push_back(arc);
	}

	void TriggerLightningGlow(int x, int y, int z, float scale, byte r, byte g, byte b)
	{
		auto& spark = *GetFreeParticle();

		spark.on = true;
		spark.spriteIndex = Objects[ID_MISC_SPRITES].meshIndex;
		spark.blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		spark.x = x;
		spark.y = y;
		spark.z = z;
		spark.xVel = 0;
		spark.yVel = 0;
		spark.zVel = 0;
		spark.sR = r;
		spark.sG = g;
		spark.sB = b;
		spark.dR = r;
		spark.dG = g;
		spark.dB = b;
		spark.life = 4;
		spark.sLife = 4;
		spark.colFadeSpeed = 2;
		spark.fadeToBlack = 0;
		spark.scalar = 3;
		spark.maxYvel = 0;
		spark.gravity = 0;
		spark.sSize =
		spark.dSize =
		spark.size = scale + Random::GenerateInt(0, 4);
		spark.flags = SP_DEF | SP_SCALE;
	}

	void SpawnHelicalLaser(const Vector3& origin, const Vector3& target)
	 {
		static constexpr auto flags = LI_THININ | LI_THINOUT;

		auto laser = HelicalLaser();

		laser.NumSegments = HELICAL_LASER_SEGMENTS_NUM_MAX;
		laser.Origin = origin;
		laser.Target = target;
		laser.Orientation2D = Random::GenerateAngle();
		laser.LightPosition = origin;
		laser.Color = Vector4(0.0f, 1.0f, 0.375f, 1.0f); // TODO: Check start opacity.
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

		TriggerLightning(&origin2, &target2, 1, 0, laser.g, laser.b, 20, flags, 19, 5);
		TriggerLightning(&origin2, &target2, 1, 110, 255, 250, 20, flags, 4, 5);
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
			if (arc.life <= 0.0f)
				continue;

			// If/when this behaviour is changed, modify AddLightningArc accordingly.
			arc.life -= 2.0f;
			if (arc.life > 0.0f)
			{
				// TODO: Find a better way for this.
				auto* posPtr = (Vector3*)&arc.pos2;
				for (auto& interpPos : arc.interpolation)
				{
					*posPtr += interpPos * 2;
					interpPos -= (interpPos / 16);

					posPtr++;
				}
			}
		}

		// Despawn inactive effects.
		ElectricArcs.erase(
			std::remove_if(
				ElectricArcs.begin(), ElectricArcs.end(),
				[](const ElectricArc& arc) { return (arc.life <= 0.0f); }), ElectricArcs.end());
	}

	void CalculateElectricArcSpline(const std::array<Vector3, 6>& posArray, std::array<Vector3, 1024>& bufferArray, const ElectricArc& arc)
	{
		int bufferIndex = 0;

		bufferArray[bufferIndex] = posArray[0];
		bufferIndex++;

		// Splined arc.
		if (arc.flags & LI_SPLINE)
		{
			int interpStep = 65536 / ((arc.segments * 3) - 1);
			int alpha = interpStep;

			if (((arc.segments * 3) - 2) > 0)
			{
				for (int i = (arc.segments * 3) - 2; i > 0; i--)
				{
					auto spline = ElectricArcSpline(alpha, &posArray[0], posArray.size());
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
			auto pos = posArray[0] + deltaPos + Vector3(
				fmod(Random::GenerateInt(), arc.amplitude * 2),
				fmod(Random::GenerateInt(), arc.amplitude * 2),
				fmod(Random::GenerateInt(), arc.amplitude * 2)) -
				Vector3(arc.amplitude);

			if (((arc.segments * 3) - 2) > 0)
			{
				for (int i = (arc.segments * 3) - 2; i > 0; i--)
				{
					bufferArray[bufferIndex] = pos;
					bufferIndex++;

					pos += deltaPos + Vector3(
						fmod(Random::GenerateInt(), arc.amplitude * 2),
						fmod(Random::GenerateInt(), arc.amplitude * 2),
						fmod(Random::GenerateInt(), arc.amplitude * 2)) -
						Vector3(arc.amplitude);
				}
			}
		}

		bufferArray[bufferIndex] = posArray[5];
	}

	// BIG TODO: Make a family of Bezier, B-Spline, and Catmull-Rom curve classes.

	// 4-point Catmull-Rom spline interpolation.
	// NOTE: Alpha is in range [0, 65536] rather than [0, 1].
	// Function takes pointer to array of 6 knots and determines subset of 4
	// using passed alpha value. numKnots is always 6.
	Vector3 ElectricArcSpline(int alpha, const Vector3* knots, int numKnots)
	{
		alpha *= numKnots - 3;

		int span = alpha / pow(2, 16);
		if (span >= (numKnots - 3))
			span = numKnots - 4;

		float alphaNorm = (alpha - (65536 * span)) / pow(2, 16);

		auto* knotPtr = &knots[span];

		auto point1 = knotPtr[1] + (knotPtr[1] / 2) - (knotPtr[2] / 2) - knotPtr[2] + (knotPtr[3] / 2) + ((-knotPtr[0] - Vector3::One) / 2);
		auto ret = point1 * alphaNorm;
		auto point2 = ret + Vector3(2.0f) * knotPtr[2] - 2 * knotPtr[1] - (knotPtr[1] / 2) - (knotPtr[3] / 2) + knotPtr[0];
		ret = point2 * alphaNorm;
		auto point3 = ret + (knotPtr[2] / 2) + ((-knotPtr[0] - Vector3::One) / 2);
		ret = point3 * alphaNorm;

		return (ret + knotPtr[1]);
	}

	// More standard version. Adopt this in place of the above.
	Vector3 CatmullRomSpline(float alpha, const std::array<Vector3, 4>& knots)
	{
		auto knot0 = knots[0];
		auto knot1 = knots[1];
		auto knot2 = knots[2];
		auto knot3 = knots[3];

		auto point1 = knot1 + ((knot2 - knot0) * (1 / 6.0f));
		auto point2 = knot2;
		auto point3 = knot2 + ((knot3 - knot1) * (-1 / 6.0f));
		auto point4 = knot3;

		return (((point2 * 2) + (point3 - point1) * alpha) +
			   (((point1 * 2) - (point2 * 5) + (point3 * 4) - point4) * SQUARE(alpha)) +
			   (((point1 * -1) + (point2 * 3) - (point3 * 3) + point4) * CUBE(alpha)));
	}
}
