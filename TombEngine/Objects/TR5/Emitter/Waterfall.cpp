#include "framework.h"
#include "Objects/TR5/Emitter/Waterfall.h"

#include "Game/collision/collide_room.h"
#include "Game/collision/Point.h"
#include "Game/effects/effects.h"
#include "Game/camera.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Objects/Utils/object_helper.h"
#include "Specific/clock.h"
#include "Renderer/Renderer.h"

using namespace TEN::Collision::Point;
using namespace TEN::Math;
using TEN::Renderer::g_Renderer;
	
const auto WATERFALL_MAX_LIFE = 100;
const auto MAX_INVERSE_SCALE_FACTOR = 50.0f;
const auto WATERFALL_MIST_COLOR_MODIFIER = Color(20.0f, 20.0f, 20.0f);
constexpr auto WATERFALL_SPLASH = 0;
constexpr auto WATERFALL_STREAM1_SPRITE = 1;
constexpr auto WATERFALL_STREAM2_SPRITE = 2;

	void InitializeWaterfall(short itemNumber)
	{

	}

	void WaterfallControl(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (!TriggerActive(&item))
			return;

		static const int scale = 3;

		int size = 2;
		int width = 1;
		int waterHeight = 0;
		short angle = item.Pose.Orientation.y;

		if (item.TriggerFlags != 0)
		{
			width = std::clamp(int(round(item.TriggerFlags) * 100) / 2, 0, BLOCK(8));

			// Calculate the dynamic size based on TriggerFlags
			size = std::clamp(2 + (item.TriggerFlags) / 2, 15, 20);			
		}

		float cos = phd_cos(angle - ANGLE(90.0f));
		float sin = phd_sin(angle + ANGLE(90.0f));
		
		auto startColor = item.Model.Color / 4.0f  * float(SCHAR_MAX);
		auto endColor = item.Model.Color / 8.0f * float(UCHAR_MAX);

		// Calculate the inverse scale factor based on size
		float inverseScaleFactor = MAX_INVERSE_SCALE_FACTOR / size;

		// Adjust step by multiplying it with the inverse scale factor
		float step = size * scale * inverseScaleFactor;

		int currentStep = 0;

		while (true)
		{
			int offset = (step * currentStep) + Random::GenerateInt(-32, 32);

			if (offset > width)
				break;

			for (int sign = -1; sign <= 1; sign += 2)
			{
				if (Random::GenerateInt(0, 100) > std::clamp((width / 100) * 3, 30, 80))
				{
					auto* spark = GetFreeParticle();

					spark->on = true;
					spark->roomNumber = item.RoomNumber;
					spark->friction = -2;
					spark->xVel = (BLOCK(0.2f) * cos);
					spark->yVel = 16 - (GetRandomControl() & 0xF);
					spark->zVel = (BLOCK(0.2f) * sin);

					spark->x = offset * sign * sin + Random::GenerateInt(-8, 8) + item.Pose.Position.x;
					spark->y = Random::GenerateInt(0, 16) + item.Pose.Position.y - 8;
					spark->z = offset * sign * cos + Random::GenerateInt(-8, 8) + item.Pose.Position.z;

					auto orient = EulerAngles(item.Pose.Orientation.x - ANGLE(90.0f), item.Pose.Orientation.y, item.Pose.Orientation.z);
					auto orient2 = EulerAngles(item.Pose.Orientation.x, item.Pose.Orientation.y, item.Pose.Orientation.z);
					auto dir = orient.ToDirection();
					
					auto origin = GameVector(Vector3(spark->x, spark->y, spark->z), item.RoomNumber);
					auto origin2 = Geometry::TranslatePoint(Vector3(spark->x, spark->y, spark->z), orient2, BLOCK(0.3));

					auto pointColl = GetPointCollision(origin2, origin.RoomNumber, dir, BLOCK(8));
				
					int relFloorHeight = pointColl.GetFloorHeight() - spark->y;
				
					spark->targetPos = GameVector(origin2.x, origin2.y + relFloorHeight, origin2.z, pointColl.GetRoomNumber());

					if (TestEnvironment(ENV_FLAG_WATER, spark->targetPos.ToVector3i(), spark->roomNumber) ||
						TestEnvironment(ENV_FLAG_SWAMP, spark->targetPos.ToVector3i(), spark->roomNumber))
					{
						 waterHeight = GetWaterDepth(spark->targetPos.x, spark->targetPos.y, spark->targetPos.z, item.RoomNumber);
					}
						
					spark->targetPos.y -= waterHeight;

					char colorOffset = (Random::GenerateInt(-8, 8));
					spark->sR = std::clamp(int(startColor.x) + colorOffset, 0, UCHAR_MAX);
					spark->sG = std::clamp(int(startColor.y) + colorOffset, 0, UCHAR_MAX);
					spark->sB = std::clamp(int(startColor.z) + colorOffset, 0, UCHAR_MAX);
					spark->dR = std::clamp(int(endColor.x) + colorOffset, 0, UCHAR_MAX);
					spark->dG = std::clamp(int(endColor.y) + colorOffset, 0, UCHAR_MAX);
					spark->dB = std::clamp(int(endColor.z) + colorOffset, 0, UCHAR_MAX);
					spark->roomNumber = pointColl.GetRoomNumber();
					spark->colFadeSpeed = 2;
					spark->blendMode = BlendMode::Additive;

					spark->gravity = (relFloorHeight / 2) / FPS; // Adjust gravity based on relative floor height
					spark->life = spark->sLife = WATERFALL_MAX_LIFE;
					spark->fxObj = ID_WATERFALL_EMITTER;
					spark->fadeToBlack = 0;

					spark->rotAng = GetRandomControl() & 0xFFF;
					spark->scalar = item.TriggerFlags < 10 ? Random::GenerateInt(2, 4) : Random::GenerateInt(3, 5);
					spark->maxYvel = 0;
					spark->rotAdd = Random::GenerateInt(-16, 16);

					spark->sSize = spark->size = (item.TriggerFlags < 10 ? Random::GenerateFloat(40.0f, 51.0f) : Random::GenerateFloat(49.0f, 87.0f)) / 2;
					spark->dSize = item.TriggerFlags < 10 ? Random::GenerateFloat(40.0f, 51.0f) : Random::GenerateFloat(98.0f, 174.0f);

					spark->spriteIndex = Objects[ID_WATERFALL_SPRITES].meshIndex + (Random::GenerateInt(0, 100) > 40 ? WATERFALL_STREAM2_SPRITE : WATERFALL_SPLASH);
					spark->flags = SP_SCALE | SP_DEF | SP_ROTATE;
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

	void TriggerWaterfallEmitterMist(const Vector3& pos, short room, short scalar, short size, Color color)
	{
		short angle = pos.y ;

		float cos = phd_cos(angle);
		float sin = phd_sin(angle);

		int maxPosX =  sin + pos.x;
		int maxPosZ =  cos + pos.z;
		int minPosX =  sin + pos.x;
		int minPosZ =  cos + pos.z;

		auto colorOffset = Color(40.0f, 40.0f, 40.0f);

		auto startColor = (color + colorOffset);
		auto endColor = (color + colorOffset);
		
		auto* spark = GetFreeParticle();
		
		spark->on = true;
		spark->roomNumber = room;
		char colorVariation = (Random::GenerateInt(-8, 8));
		spark->sR =  std::clamp(int(startColor.x) + colorVariation, 0, SCHAR_MAX);
		spark->sG =  std::clamp(int(startColor.y) + colorVariation, 0, SCHAR_MAX);
		spark->sB = std::clamp(int(startColor.z) + colorVariation, 0, SCHAR_MAX);
		spark->dR = std::clamp(int(endColor.x) + colorVariation, 0, SCHAR_MAX);
		spark->dG = std::clamp(int(endColor.y) + colorVariation, 0, SCHAR_MAX);
		spark->dB = std::clamp(int(endColor.z) + colorVariation, 0, SCHAR_MAX);

		spark->colFadeSpeed = 1;
		spark->blendMode = BlendMode::Additive;
		spark->life = spark->sLife = 8;
		spark->fadeToBlack = spark->life ;

		spark->x = cos * Random::GenerateInt(-12, 12) + pos.x;
		spark->y = Random::GenerateInt(0, 16) + pos.y - 8;
		spark->z = sin*  Random::GenerateInt(-12, 12) + pos.z;

		spark->xVel = 0;
		spark->yVel = -Random::GenerateInt(63, 64);
		spark->zVel = 0;

		spark->friction = 3;
		spark->rotAng = GetRandomControl() & 0xFFF;

		auto scale = scalar = 3 ? scalar - 1 : scalar + 4;
		scale = std::clamp(int(scale), 2, 9);
		spark->scalar = scale;
		
		spark->rotAdd = Random::GenerateInt(-16, 16);
		spark->gravity = spark->maxYvel = -64;

		float size1 = (GetRandomControl() & 8) + (size * 4) + scalar;
		spark->size = spark->sSize = size1 / 4;
		spark->dSize = size1;

		spark->spriteIndex = Objects[ID_WATERFALL_SPRITES].meshIndex + (Random::GenerateInt(0, 100) > 40 ? WATERFALL_STREAM2_SPRITE : WATERFALL_STREAM1_SPRITE);
		spark->flags = SP_SCALE | SP_DEF | SP_ROTATE;		
	}
