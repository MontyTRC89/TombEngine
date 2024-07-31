#include "framework.h"
#include "Objects/TR5/Emitter/Waterfall.h"

#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/Point.h"
#include "Game/effects/effects.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Objects/Utils/object_helper.h"
#include "Renderer/Renderer.h"
#include "Specific/clock.h"

using namespace TEN::Collision::Point;
using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

namespace TEN::Effects::WaterfallEmitter
{
	constexpr auto WATERFALL_LIFE_MAX				  = 100;
	constexpr auto WATERFALL_INVERSE_SCALE_FACTOR_MAX = 50.0f;
	constexpr auto WATERFALL_MIST_COLOR_MODIFIER	  = Color(20.0f, 20.0f, 20.0f);

	constexpr auto WATERFALL_SPLASH_SPRITE_ID	= 0;
	constexpr auto WATERFALL_STREAM_1_SPRITE_ID = 1;
	constexpr auto WATERFALL_STREAM_2_SPRITE_ID = 2;

	void InitializeWaterfall(short itemNumber)
	{
		// TODO
	}

	void ControlWaterfall(short itemNumber)
	{
		constexpr auto SCALE = 3.0f;

		auto& item = g_Level.Items[itemNumber];

		if (!TriggerActive(&item))
			return;

		int size = 2;
		int width = 1;
		int waterHeight = 0;
		short angle = item.Pose.Orientation.y;

		if (item.TriggerFlags != 0)
		{
			width = std::clamp(int(round(item.TriggerFlags) * 100) / 2, 0, BLOCK(8));

			// Calculate dynamic size based on TriggerFlags.
			size = std::clamp(2 + (item.TriggerFlags) / 2, 15, 20);
		}

		float cos = phd_cos(angle - ANGLE(90.0f));
		float sin = phd_sin(angle + ANGLE(90.0f));

		auto startColor = (item.Model.Color / 4) * SCHAR_MAX; // ??
		auto endColor = (item.Model.Color / 8) * UCHAR_MAX;

		// Calculate inverse scale factor based on size.
		float inverseScaleFactor = WATERFALL_INVERSE_SCALE_FACTOR_MAX / size;

		// Adjust step by multiplying with inverse scale factor.
		float step = (size * SCALE) * inverseScaleFactor;

		int currentStep = 0;
		int offset = 0;
		while (offset <= width)
		{
			// TODO: Constants.
			offset = (step * currentStep) + Random::GenerateInt(-32, 32);

			for (int sign = -1; sign <= 1; sign += 2) // ??
			{
				// TODO: Use Random::TestProbability().
				if (Random::GenerateInt(0, 100) > std::clamp((width / 100) * 3, 30, 80))
				{
					auto& part = *GetFreeParticle();

					part.on = true;
					part.roomNumber = item.RoomNumber;
					part.friction = -2;
					part.xVel = BLOCK(0.2f) * cos;
					part.yVel = 16 - (GetRandomControl() & 0xF);
					part.zVel = BLOCK(0.2f) * sin;

					// TODO: Constants.
					part.x = ((offset * sign) * sin) + Random::GenerateInt(-8, 8) + item.Pose.Position.x;
					part.y = Random::GenerateInt(0, 16) + item.Pose.Position.y - 8;
					part.z = ((offset * sign) * cos) + Random::GenerateInt(-8, 8) + item.Pose.Position.z;

					auto orient = EulerAngles(item.Pose.Orientation.x - ANGLE(90.0f), item.Pose.Orientation.y, item.Pose.Orientation.z);
					auto orient2 = EulerAngles(item.Pose.Orientation.x, item.Pose.Orientation.y, item.Pose.Orientation.z);
					auto dir = orient.ToDirection();

					auto origin = GameVector(Vector3(part.x, part.y, part.z), item.RoomNumber);
					auto origin2 = Geometry::TranslatePoint(Vector3(part.x, part.y, part.z), orient2, BLOCK(0.3));

					auto pointColl = GetPointCollision(origin2, origin.RoomNumber, dir, BLOCK(8));

					int relFloorHeight = pointColl.GetFloorHeight() - part.y;

					part.targetPos = GameVector(origin2.x, origin2.y + relFloorHeight, origin2.z, pointColl.GetRoomNumber());

					if (TestEnvironment(ENV_FLAG_WATER, part.targetPos.ToVector3i(), part.roomNumber) ||
						TestEnvironment(ENV_FLAG_SWAMP, part.targetPos.ToVector3i(), part.roomNumber))
					{
						waterHeight = GetWaterDepth(part.targetPos.x, part.targetPos.y, part.targetPos.z, item.RoomNumber);
					}

					part.targetPos.y -= waterHeight;

					char colorOffset = Random::GenerateInt(-8, 8);
					part.sR = std::clamp((int)startColor.x + colorOffset, 0, UCHAR_MAX);
					part.sG = std::clamp((int)startColor.y + colorOffset, 0, UCHAR_MAX);
					part.sB = std::clamp((int)startColor.z + colorOffset, 0, UCHAR_MAX);
					part.dR = std::clamp((int)endColor.x + colorOffset, 0, UCHAR_MAX);
					part.dG = std::clamp((int)endColor.y + colorOffset, 0, UCHAR_MAX);
					part.dB = std::clamp((int)endColor.z + colorOffset, 0, UCHAR_MAX);
					part.roomNumber = pointColl.GetRoomNumber();
					part.colFadeSpeed = 2;
					part.blendMode = BlendMode::Additive;

					part.gravity = (relFloorHeight / 2) / FPS; // Adjust gravity based on relative floor height.
					part.life =
					part.sLife = WATERFALL_LIFE_MAX;
					part.fxObj = ID_WATERFALL_EMITTER;
					part.fadeToBlack = 0;

					// TODO: Magic.
					part.rotAng = GetRandomControl() & 0xFFF;
					part.scalar = item.TriggerFlags < 10 ? Random::GenerateInt(2, 4) : Random::GenerateInt(3, 5);
					part.maxYvel = 0;
					part.rotAdd = Random::GenerateInt(-16, 16);

					part.sSize =
					part.size = (item.TriggerFlags < 10 ? Random::GenerateFloat(40.0f, 51.0f) : Random::GenerateFloat(49.0f, 87.0f)) / 2;
					part.dSize = item.TriggerFlags < 10 ? Random::GenerateFloat(40.0f, 51.0f) : Random::GenerateFloat(98.0f, 174.0f);

					part.SpriteSeqID = ID_WATERFALL;
					part.SpriteID = Random::TestProbability(1 / 2.0f) ? WATERFALL_STREAM_2_SPRITE_ID : WATERFALL_SPLASH_SPRITE_ID;
					part.flags = SP_SCALE | SP_DEF | SP_ROTATE;
				}

				if (sign == 1)
				{
					currentStep++;
					if (currentStep == 1)
						break;
				}
			}
		}
	}

	void SpawnWaterfallMist(const Vector3& pos, int roomNumber, float scalar, float size, const Color& color)
	{
		constexpr auto SPHERE_RADIUS = BLOCK(0.01f);

		auto& part = *GetFreeParticle();

		short angle = pos.y; // TODO: Not an angle, how does it work?

		// TODO: Matrix math.
		float cos = phd_cos(angle);
		float sin = phd_sin(angle);

		int maxPosX = sin + pos.x;
		int maxPosZ = cos + pos.z;
		int minPosX = sin + pos.x;
		int minPosZ = cos + pos.z;

		auto colorOffset = Color(40.0f); // make constant.

		auto startColor = (color + colorOffset);
		auto endColor = (color + colorOffset);

		// TODO: Generate normal color representation, then convert to legacy.
		part.on = true;
		part.SpriteSeqID = ID_WATERFALL;
		part.SpriteID = Random::TestProbability(1 / 2.0f) ? WATERFALL_STREAM_1_SPRITE_ID : WATERFALL_STREAM_2_SPRITE_ID;
		part.roomNumber = roomNumber;
		char colorVariation = (Random::GenerateInt(-8, 8)); // TODO: Why char?
		part.sR = std::clamp((int)startColor.x + colorVariation, 0, SCHAR_MAX);
		part.sG = std::clamp((int)startColor.y + colorVariation, 0, SCHAR_MAX);
		part.sB = std::clamp((int)startColor.z + colorVariation, 0, SCHAR_MAX);
		part.dR = std::clamp((int)endColor.x + colorVariation, 0, SCHAR_MAX);
		part.dG = std::clamp((int)endColor.y + colorVariation, 0, SCHAR_MAX);
		part.dB = std::clamp((int)endColor.z + colorVariation, 0, SCHAR_MAX);

		part.colFadeSpeed = 1;
		part.blendMode = BlendMode::Additive;
		part.life =
		part.sLife = 8;
		part.fadeToBlack = part.life;

		part.x = cos * Random::GenerateInt(-12, 12) + pos.x;
		part.y = Random::GenerateInt(0, 16) + pos.y - 8;
		part.z = sin * Random::GenerateInt(-12, 12) + pos.z;

		part.xVel = 0;
		part.yVel = -Random::GenerateInt(63, 64);
		part.zVel = 0;

		part.friction = 3;
		part.rotAng = Random::GenerateAngle();

		// ??
		auto scale =
		scalar = 3.0f ? (scalar - 1.0f) : (scalar + 4.0f);
		scale = std::clamp(int(scale), 2, 9);
		part.scalar = scale;

		part.rotAdd = Random::GenerateInt(-16, 16);
		part.gravity = part.maxYvel = -64;

		float size1 = (GetRandomControl() & 8) + (size * 4) + scalar;
		part.size = part.sSize = size1 / 4;
		part.dSize = size1;

		part.flags = SP_SCALE | SP_DEF | SP_ROTATE;
	}
}
