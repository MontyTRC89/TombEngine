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

// NOTES
// item.TriggetFlags: Waterfall width. 1 unit = BLOCK(1 / 8.0f).

namespace TEN::Effects::WaterfallEmitter
{
	constexpr auto WATERFALL_LIFE_MAX			 = 100;
	constexpr auto WATERFALL_MIST_COLOR_MODIFIER = Color(20.0f, 20.0f, 20.0f);

	constexpr auto WATERFALL_SPLASH_SPRITE_ID	= 0;
	constexpr auto WATERFALL_STREAM_1_SPRITE_ID = 1;
	constexpr auto WATERFALL_STREAM_2_SPRITE_ID = 2;

	void InitializeWaterfall(short itemNumber)
	{
		// TODO
	}

	void ControlWaterfall(short itemNumber)
	{
		constexpr auto SCALE		= 3.0f;
		constexpr auto SPAWN_RADIUS = BLOCK(1 / 16.0f);

		auto& item = g_Level.Items[itemNumber];
		if (!TriggerActive(&item))
			return;

		float waterfallWidth = std::max(BLOCK(item.TriggerFlags / 8.0f), BLOCK(0.5f));

		auto vel = item.Pose.Orientation.ToDirection() * BLOCK(0.2f);

		auto startColor = (item.Model.Color / 4) * SCHAR_MAX;
		auto endColor = (item.Model.Color / 8) * UCHAR_MAX;

		// Spawn particles.
		unsigned int partCount = (int)round(waterfallWidth / BLOCK(0.5f));
		for (int i = 0; i < partCount; i++)
		{
			auto& part = *GetFreeParticle();

			auto rotMatrix = item.Pose.Orientation.ToRotationMatrix();
			auto relOffset = Vector3(Random::GenerateFloat(-waterfallWidth / 2, waterfallWidth / 2), 0.0f, 0.0f);
			auto offset = Vector3::Transform(relOffset, rotMatrix);
			auto sphere = BoundingSphere(offset, SPAWN_RADIUS);
			offset = Random::GeneratePointInSphere(sphere);
			auto pos = item.Pose.Position.ToVector3() + offset;

			vel.y = Random::GenerateFloat(0.0f, 16.0f);

			part.on = true;
			part.SpriteSeqID = ID_WATERFALL;
			part.SpriteID = Random::TestProbability(1 / 2.0f) ? WATERFALL_STREAM_2_SPRITE_ID : WATERFALL_SPLASH_SPRITE_ID;
			part.x = pos.x;
			part.y = pos.y;
			part.z = pos.z;
			part.roomNumber = item.RoomNumber;
			part.xVel = vel.x;
			part.yVel = vel.y;
			part.zVel = vel.z;
			part.friction = -2;

			auto pointColl = GetPointCollision(pos, item.RoomNumber, item.Pose.Orientation.y, BLOCK(0.3f));
			part.targetPos = GameVector(pointColl.GetPosition().x, pointColl.GetFloorHeight(), pointColl.GetPosition().z, pointColl.GetRoomNumber());
			if (TestEnvironment(ENV_FLAG_WATER, part.targetPos.ToVector3i(), part.roomNumber) ||
				TestEnvironment(ENV_FLAG_SWAMP, part.targetPos.ToVector3i(), part.roomNumber))
			{
				part.targetPos.y = pointColl.GetWaterSurfaceHeight();
			}

			// TODO: Offset colour earlier.
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
			part.gravity = 120;
			part.life =
			part.sLife = WATERFALL_LIFE_MAX;
			part.fxObj = ID_WATERFALL_EMITTER;
			part.fadeToBlack = 0;
			part.rotAng = Random::GenerateAngle();
			part.rotAdd = Random::GenerateAngle(ANGLE(-0.1f), ANGLE(0.1f));
			part.scalar = item.TriggerFlags < 10 ? Random::GenerateInt(2, 4) : Random::GenerateInt(3, 5);
			part.maxYvel = 0;
			part.sSize =
			part.size = (item.TriggerFlags < 10 ? Random::GenerateFloat(40.0f, 51.0f) : Random::GenerateFloat(49.0f, 87.0f)) / 2;
			part.dSize = item.TriggerFlags < 10 ? Random::GenerateFloat(40.0f, 51.0f) : Random::GenerateFloat(98.0f, 174.0f);
			part.flags = SP_SCALE | SP_DEF | SP_ROTATE;
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

		part.SpriteSeqID = ID_WATERFALL_SPRITES;
		part.SpriteID = Random::TestProbability(1 / 2.0f) ? WATERFALL_STREAM_1_SPRITE_ID : WATERFALL_STREAM_2_SPRITE_ID;

		part.on = true;

		part.x = cos * Random::GenerateInt(-12, 12) + pos.x;
		part.y = Random::GenerateInt(0, 16) + pos.y - 8;
		part.z = sin * Random::GenerateInt(-12, 12) + pos.z;
		part.roomNumber = roomNumber;

		// TODO: Generate normal color representation, then convert to legacy.
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

		part.xVel = 0;
		part.yVel = -Random::GenerateInt(63, 64);
		part.zVel = 0;

		part.friction = 3;
		part.rotAng = Random::GenerateAngle();

		auto scale = (scalar == 3.0f) ? (scalar - 1.0f) : (scalar + 4.0f);
		scale = std::clamp(int(scale), 2, 9);
		part.scalar = scale;

		part.rotAdd = Random::GenerateInt(-16, 16);
		part.gravity =
		part.maxYvel = -64;

		float size1 = (GetRandomControl() & 8) + (size * 4) + scalar;
		part.size =
		part.sSize = size1 / 4;
		part.dSize = size1;

		part.flags = SP_SCALE | SP_DEF | SP_ROTATE;
	}
}
