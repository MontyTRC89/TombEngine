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

    enum WaterfallItemFlags
    {
        Velocity,
        WaterfallSpriteScale,
        Density,
        MistSpriteScale,
        Sound
    };

	constexpr auto WATERFALL_LIFE_MAX			= 100;
	constexpr auto WATERFALL_SPLASH_SPRITE_ID	= 0;
	constexpr auto WATERFALL_STREAM_1_SPRITE_ID = 1;
	constexpr auto WATERFALL_STREAM_2_SPRITE_ID = 2;

	void InitializeWaterfall(short itemNumber)
	{
        auto& item = g_Level.Items[itemNumber];

        //Customize x and z vel:
        item.ItemFlags[WaterfallItemFlags::Velocity] = 100;

        //Customize Waterfall sprite scale
        item.ItemFlags[WaterfallItemFlags::WaterfallSpriteScale] = 3;

        //Customize density
        item.ItemFlags[WaterfallItemFlags::Density] = 120;

        //Customize Waterfallmist sprite scale
        item.ItemFlags[WaterfallItemFlags::MistSpriteScale] = 3;

        //Customize Waterfallmist sprite scale
        item.ItemFlags[WaterfallItemFlags::Sound] = 0;
	}

    void ControlWaterfall(short itemNumber)
    {
        auto& item = g_Level.Items[itemNumber];

        if (!TriggerActive(&item))
            return;

        float customVel = item.ItemFlags[WaterfallItemFlags::Velocity] / 256.0f;

        short scale = item.ItemFlags[WaterfallItemFlags::WaterfallSpriteScale] ;
        scale = Random::GenerateInt(scale, scale + 2);

        float density = item.TriggerFlags < 5 ? std::clamp(int(item.ItemFlags[WaterfallItemFlags::Density]), 10, 256) : std::clamp(int(item.ItemFlags[WaterfallItemFlags::Density]), 80, 256);
        density = density / 256.0f;

        if (!item.ItemFlags[WaterfallItemFlags::Sound])
            SoundEffect(SFX_TR4_WATERFALL_LOOP, &item.Pose);

        float waterfallWidth = std::max(CLICK(float(item.TriggerFlags)), CLICK(0.1f));
        auto vel = item.Pose.Orientation.ToDirection() * BLOCK(customVel);

        auto startColor = (item.Model.Color / 4) * SCHAR_MAX;
        auto endColor = (item.Model.Color / 8) * UCHAR_MAX;

        // Spawn particles.
        unsigned int partCount = (int)round(waterfallWidth / BLOCK(density));
        for (int i = 0; i < partCount; i++)
        {
            auto& part = *GetFreeParticle();

            auto rotMatrix = item.Pose.Orientation.ToRotationMatrix();
            auto relOffset = Vector3(Random::GenerateFloat(-waterfallWidth / 2.0f, waterfallWidth / 2.0f), 0.0f, 0.0f);
            auto offset = Vector3::Transform(relOffset, rotMatrix);
            auto pos = item.Pose.Position.ToVector3() + offset;
            auto orient2 = EulerAngles(item.Pose.Orientation.x, item.Pose.Orientation.y, item.Pose.Orientation.z);
            auto origin2 = Geometry::TranslatePoint(Vector3(pos.x, pos.y, pos.z), orient2, BLOCK(customVel));

            vel.y = Random::GenerateFloat(0.0f, 16.0f);

            part.on = true;
            part.SpriteSeqID = ID_WATERFALL;
            part.SpriteID = Random::TestProbability(1 / 2.2f) ? WATERFALL_STREAM_2_SPRITE_ID : WATERFALL_SPLASH_SPRITE_ID;
            part.x = pos.x;
            part.y = pos.y;
            part.z = pos.z;
            part.roomNumber = item.RoomNumber;
            part.xVel = vel.x;
            part.yVel = vel.y;
            part.zVel = vel.z;
            part.friction = -2;
            part.itemNumber = itemNumber;

            auto pointColl = GetPointCollision(origin2, item.RoomNumber, item.Pose.Orientation.y, BLOCK(customVel));

            int relFloorHeight = pointColl.GetFloorHeight() - part.y;
            part.targetPos = Vector4(origin2.x, origin2.y + relFloorHeight, origin2.z, itemNumber);

            // if targetpos is in a wall, calculate targetpos from the main item coordinates so the particles still spawn.
            if (pointColl.GetSector().IsWall(part.targetPos.x, part.targetPos.z))
            {
                pointColl = GetPointCollision(origin2, item.RoomNumber, item.Pose.Orientation.y, BLOCK(0.0f));
                relFloorHeight = pointColl.GetFloorHeight() - part.y;
                origin2 = Geometry::TranslatePoint(Vector3(pos.x, pos.y, pos.z), orient2, BLOCK(0.0f));
                //set the x and z values to 0 to prevent it from spawning a waterfallmist within the wall. Also sets the right height.
                part.targetPos = Vector4(origin2.x, origin2.y + relFloorHeight, origin2.z, itemNumber);

                //Stop spawning particles if there is a wall directly at the spawnpoint
                if (pointColl.GetSector().IsWall(pos.x, pos.z))
                {
                    part.on = false;
                    continue;
                }
            }

            if (TestEnvironment(ENV_FLAG_WATER, Vector3i(part.targetPos.x, part.targetPos.y, part.targetPos.z), part.roomNumber) ||
                TestEnvironment(ENV_FLAG_SWAMP, Vector3i(part.targetPos.x, part.targetPos.y, part.targetPos.z), part.roomNumber))
            {
                part.targetPos.y = pointColl.GetWaterSurfaceHeight();
            }

            //Debug
            //g_Renderer.AddDebugLine(origin2, Vector3(part.targetPos.x, part.targetPos.y, part.targetPos.z) , Vector4(1, 0, 0, 1));

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
            part.life = part.sLife = WATERFALL_LIFE_MAX;
            part.fadeToBlack = 0;
            part.rotAng = Random::GenerateAngle();
            part.rotAdd = Random::GenerateAngle(ANGLE(-0.1f), ANGLE(0.1f));
            part.scalar = scale;// item.TriggerFlags < 10 ? Random::GenerateInt(2, 4) : Random::GenerateInt(3, 5);
            part.maxYvel = 0;
            part.sSize = part.size = (item.TriggerFlags < 10 ? Random::GenerateFloat(40.0f, 51.0f) : Random::GenerateFloat(49.0f, 87.0f)) / 2;
            part.dSize = item.TriggerFlags < 10 ? Random::GenerateFloat(40.0f, 51.0f) : Random::GenerateFloat(98.0f, 174.0f);
            part.flags = SP_SCALE | SP_DEF | SP_ROTATE;
        }
    }

	void SpawnWaterfallMist(const Vector4& pos, int roomNumber, float scalar, float size, const Color& color)
	{
		auto& part = *GetFreeParticle();

		auto colorOffset = Color(40.0f, 40.0f, 40.0f); // make constant. Color is Vector 4. 4th value should not be changed.

		auto startColor = (color + colorOffset);
		auto endColor = (color + colorOffset);

		part.SpriteSeqID = ID_WATERFALL;
		part.SpriteID = Random::TestProbability(1 / 2.0f) ? WATERFALL_STREAM_2_SPRITE_ID : WATERFALL_STREAM_1_SPRITE_ID;

		part.on = true;

        part.PrevX = pos.x;
        part.PrevY = pos.y;
        part.PrevZ = pos.z;
		part.x = pos.x;
		part.y = Random::GenerateInt(-16, 0) + pos.y;
		part.z = pos.z;
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
        part.scalar = scalar;
        part.targetPos = Vector4::Zero;

		part.rotAdd = Random::GenerateInt(-16, 16);
		part.gravity = 0;
		part.maxYvel = 0;

		float size1 = (GetRandomControl() & 8) + size ;
		part.size =
		part.sSize = size1 / 4 ;
		part.dSize = size1;

		part.flags = SP_SCALE | SP_DEF | SP_ROTATE;
	}
}
