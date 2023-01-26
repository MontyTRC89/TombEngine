#include "framework.h"
#include "Game/effects/lightning.h"

#include "Game/effects/effects.h"
#include "Game/people.h"
#include "Math/Math.h"
#include "Specific/setup.h"

using namespace TEN::Math;

namespace TEN::Effects::ElectricArc
{
	static constexpr auto HELICAL_LASER_LIFE_MAX = 18.0f;

	std::vector<ElectricArc>  ElectricArcs	= {};
	std::vector<HelicalLaser> HelicalLasers = {};

	std::array<Vector3, ELECTRIC_ARC_KNOTS_SIZE>  ElectricArcKnots  = {};
	std::array<Vector3, ELECTRIC_ARC_BUFFER_SIZE> ElectricArcBuffer = {};

	// BIG TODO: Make a family of Bezier, B-Spline, and Catmull-Rom curve classes.

	// 4-point Catmull-Rom spline interpolation.
	// Function takes reference to array of knots and
	// calculates using subset of 4 determined alpha value.
	static Vector3 ElectricArcSpline(const std::array<Vector3, ELECTRIC_ARC_KNOTS_SIZE>& knots, float alpha)
	{
		alpha *= ELECTRIC_ARC_KNOTS_SIZE - 3;

		int span = alpha;
		if (span >= (ELECTRIC_ARC_KNOTS_SIZE - 3))
			span = ELECTRIC_ARC_KNOTS_SIZE - 4;

		float alphaNorm = alpha - span; // What?

		// Determine subset of 4 knots.
		const auto& knot0 = knots[span];
		const auto& knot1 = knots[span + 1];
		const auto& knot2 = knots[span + 2];
		const auto& knot3 = knots[span + 3];

		auto point1 = knot1 + (knot1 / 2) - (knot2 / 2) - knot2 + (knot3 / 2) + ((-knot0 - Vector3::One) / 2);
		auto ret = point1 * alphaNorm;
		auto point2 = ret + Vector3(2.0f) * knot2 - 2 * knot1 - (knot1 / 2) - (knot3 / 2) + knot0;
		ret = point2 * alphaNorm;
		auto point3 = ret + (knot2 / 2) + ((-knot0 - Vector3::One) / 2);
		ret = point3 * alphaNorm;

		return (ret + knot1);
	}

	// More standard version. Adopt this in place of the above.
	static Vector3 CatmullRomSpline(float alpha, const std::array<Vector3, 4>& knots)
	{
		auto point1 = knots[1] + ((knots[2] - knots[0]) * (1 / 6.0f));
		auto point2 = knots[2];
		auto point3 = knots[2] + ((knots[3] - knots[1]) * (-1 / 6.0f));
		auto point4 = knots[3];

		auto spline = ((point2 * 2) + (point3 - point1) * alpha) +
					  (((point1 * 2) - (point2 * 5) + (point3 * 4) - point4) * SQUARE(alpha)) +
					  (((point1 * -1) + (point2 * 3) - (point3 * 3) + point4) * CUBE(alpha));
		return spline;
	}

	// TODO: Pass const Vector4& for color.
	void SpawnElectricArc(const Vector3& origin, const Vector3& target, float amplitude, byte r, byte g, byte b, float life, int flags, float width, unsigned int numSegments)
	{
		auto arc = ElectricArc();

		arc.pos1 = origin;
		arc.pos2 = ((origin * 3) + target) / 4;
		arc.pos3 = ((target * 3) + origin) / 4;
		arc.pos4 = target;
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

	void SpawnElectricArcGlow(const Vector3& pos, float scale, byte r, byte g, byte b)
	{
		auto& spark = *GetFreeParticle();

		spark.on = true;
		spark.spriteIndex = Objects[ID_MISC_SPRITES].meshIndex;
		spark.blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		spark.x = pos.x;
		spark.y = pos.y;
		spark.z = pos.z;
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
		static constexpr auto SEGMENTS_NUM_MAX = 56;
		static constexpr auto COLOR			   = Vector4(0.0f, 0.375f, 1.0f, 1.0f);
		static constexpr auto LENGTH_MAX	   = BLOCK(4);
		static constexpr auto ROTATION		   = ANGLE(-10.0f);

		static constexpr auto ELECTRIC_ARC_FLAGS = LI_THININ | LI_THINOUT;

		auto laser = HelicalLaser();

		laser.NumSegments = SEGMENTS_NUM_MAX;
		laser.Origin = origin;
		laser.Target = target;
		laser.Orientation2D = Random::GenerateAngle();
		laser.LightPosition = origin;
		laser.Color = COLOR;
		laser.Life = HELICAL_LASER_LIFE_MAX;
		laser.Radius = 0.0f;
		laser.Length = 0.0f;
		laser.LengthEnd = LENGTH_MAX;
		laser.Opacity = 1.0f;
		laser.Rotation = ROTATION;

		HelicalLasers.push_back(laser);

		SpawnElectricArc(origin, target, 1, 0, laser.Color.x * UCHAR_MAX, laser.Color.z * UCHAR_MAX, 20, ELECTRIC_ARC_FLAGS, 19, 5);
		SpawnElectricArc(origin, target, 1, 110, 255, 250, 20, ELECTRIC_ARC_FLAGS, 4, 5);
		SpawnElectricArcGlow(laser.LightPosition, 0, 0, (laser.Color.x / 2) * UCHAR_MAX, (laser.Color.z / 2) * UCHAR_MAX);
	 }

	void UpdateHelicalLasers()
	{
		static constexpr auto LIFE_START_FADING = HELICAL_LASER_LIFE_MAX / 2;
		static constexpr auto NUM_TURNS = 12;

		// No active effects; return early.
		if (HelicalLasers.empty())
			return;

		for (auto& laser : HelicalLasers)
		{
			// Set to despawn.
			laser.Life -= 1.0f;
			if (laser.Life <= 0.0f)
				continue;

			// Update length. TODO
			laser.Length = laser.LengthEnd;// Lerp(laser.Length, laser.LengthEnd, 0.25f);

			// Update radius.
			laser.Radius += 0.5f;

			// Update opacity.
			float alpha = laser.Life / LIFE_START_FADING;
			laser.Opacity = Lerp(0.0f, 1.0f, alpha);

			laser.Orientation2D += laser.Rotation;
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

	void CalculateElectricArcSpline(const ElectricArc& arc, const std::array<Vector3, ELECTRIC_ARC_KNOTS_SIZE>& posArray, std::array<Vector3, ELECTRIC_ARC_BUFFER_SIZE>& bufferArray)
	{
		int bufferIndex = 0;

		bufferArray[bufferIndex] = posArray[0];
		bufferIndex++;

		// Splined arc.
		if (arc.flags & LI_SPLINE)
		{
			float interpStep = 1.0f / ((arc.segments * 3) - 1);
			float alpha = interpStep;

			if (((arc.segments * 3) - 2) > 0)
			{
				for (int i = (arc.segments * 3) - 2; i > 0; i--)
				{
					auto spline = ElectricArcSpline(posArray, alpha);
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

	void CalculateHelixSpline(const HelicalLaser& laser, std::array<Vector3, ELECTRIC_ARC_KNOTS_SIZE>& posArray, std::array<Vector3, ELECTRIC_ARC_BUFFER_SIZE>& bufferArray)
	{
		int bufferIndex = 0;

		bufferArray[bufferIndex] = posArray[0];
		bufferIndex++;

		auto origin = posArray[0];
		auto target = posArray[1];
		auto direction = target - origin;
		direction.Normalize();

		float stepLength = laser.Length / laser.NumSegments;
		float radiusStep = laser.Radius;

		// TODO: Simplify this.
		auto point = Geometry::RotatePoint(Vector3::Right, EulerAngles(direction));
		auto axisAngle = AxisAngle(direction, laser.Orientation2D);

		for (int i = 0; i < laser.NumSegments; i++)
		{
			// Increment the radius and angle.
			axisAngle.SetAngle(axisAngle.GetAngle() + ANGLE(45.0f));

			// Don't use right. Establish some other point.
			auto pos = Geometry::RotatePoint(point * radiusStep * i, axisAngle);
			bufferArray[bufferIndex] = origin + Geometry::TranslatePoint(pos, direction, stepLength * i);
			bufferIndex++;
		}

		bufferArray[bufferIndex] = posArray[1];
	}
}
