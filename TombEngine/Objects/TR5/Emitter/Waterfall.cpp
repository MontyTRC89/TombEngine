#include "framework.h"
#include "Waterfall.h"

#include "Game/collision/collide_room.h"
#include "Game/effects/effects.h"
#include "Game/camera.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Objects/Utils/object_helper.h"
#include "Specific/clock.h"
#include "Renderer/Renderer.h"

using namespace TEN::Math;
using TEN::Renderer::g_Renderer;
	
	void InitializeWaterfall(short itemNumber)
	{

	}

	void WaterfallControl(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (!TriggerActive(&item))
			return;

		static const int scale = 3;

		int size = 64;
		int width = 1;
		short angle = item.Pose.Orientation.y ;

		if (item.TriggerFlags != 0)
		{
			size = item.TriggerFlags % 100;
			width = std::clamp(int(round(item.TriggerFlags / 100) * 100) / 2, 0, BLOCK(8));
		}

		float cos = phd_cos(angle - ANGLE(90.0f));
		float sin = phd_sin(angle + ANGLE(90.0f));

		int maxPosX = width * sin + item.Pose.Position.x;
		int maxPosZ = width * cos + item.Pose.Position.z;
		int minPosX = -width * sin + item.Pose.Position.x;
		int minPosZ = -width * cos + item.Pose.Position.z;

		float fadeMin = GetParticleDistanceFade(Vector3i(minPosX, item.Pose.Position.y, minPosZ));
		float fadeMax = GetParticleDistanceFade(Vector3i(maxPosX, item.Pose.Position.y, maxPosZ));

		if ((fadeMin == 0.0f) && (fadeMin == fadeMax))
			return;

		float finalFade = ((fadeMin >= 1.0f) && (fadeMin == fadeMax)) ? 1.0f : std::max(fadeMin, fadeMax);

		auto startColor = item.Model.Color / 4.0f * finalFade * float(UCHAR_MAX);
		auto endColor = item.Model.Color / 8.0f * finalFade * float(UCHAR_MAX);

		float step = size * scale;
		int currentStep = 0;

		while (true)
		{
			int offset = (step * currentStep) + Random::GenerateInt(-32, 32);

			if (offset > width)
				break;

			for (int sign = -1; sign <= 1; sign += 2)
			{
				auto* spark = GetFreeParticle();
				spark->on = true;

				spark->xVel = (BLOCK(0.5f) * cos);
				spark->yVel = 16 - (GetRandomControl() & 0xF);// Random::GenerateInt(-44, 44);
				spark->zVel = (BLOCK(0.5f) * sin);

				spark->x = offset * sign * sin + Random::GenerateInt(-8, 8) + item.Pose.Position.x;
				spark->y = Random::GenerateInt(0, 16) + item.Pose.Position.y - 8;
				spark->z = offset * sign * cos + Random::GenerateInt(-8, 8) + item.Pose.Position.z;

				auto orient = EulerAngles(item.Pose.Orientation.x - ANGLE(90.0f), item.Pose.Orientation.y, item.Pose.Orientation.z);
				auto orient2 = EulerAngles(item.Pose.Orientation.x , item.Pose.Orientation.y , item.Pose.Orientation.z );
				auto dir2 = orient2.ToDirection();
				auto dir = orient.ToDirection();
				auto rotMatrix = orient.ToRotationMatrix();

				auto origin = GameVector(Vector3(spark->x , spark->y, spark->z ), item.RoomNumber);
				auto origin2 = Geometry::TranslatePoint(Vector3(spark->x, spark->y, spark->z), orient2, BLOCK(0.7));

				//auto pointColl = GetCollision(item, dir, 0, BLOCK(8), 0);

				auto pointColl = GetCollision(origin2, origin.RoomNumber, dir, BLOCK(8));

				int relFloorHeight = pointColl.Position.Floor - item.Pose.Position.y;

				//g_Renderer.AddDebugLine(origin2, origin2 + Vector3(0, relFloorHeight, 0), Vector4(1, 0, 0, 1));

				char colorOffset = (Random::GenerateInt(-8, 8));
				spark->sR = std::clamp(int(startColor.x) + colorOffset, 0, UCHAR_MAX);
				spark->sG = std::clamp(int(startColor.y) + colorOffset, 0, UCHAR_MAX);
				spark->sB = std::clamp(int(startColor.z) + colorOffset, 0, UCHAR_MAX);
				spark->dR = std::clamp(int(endColor.x) + colorOffset, 0, UCHAR_MAX);
				spark->dG = std::clamp(int(endColor.y) + colorOffset, 0, UCHAR_MAX);
				spark->dB = std::clamp(int(endColor.z) + colorOffset, 0, UCHAR_MAX);

				spark->colFadeSpeed = 2;
				spark->blendMode = BlendMode::Additive;
				spark->gravity = (relFloorHeight / 2) / FPS;

				// Adjust particle life based on relative floor height, speed, and gravity
				spark->life = spark->sLife = 172;
				spark->fadeToBlack = 0;

				spark->friction = -2;
				spark->rotAng = GetRandomControl() & 0xFFF;
				spark->scalar = scale;
				spark->maxYvel = 0;
				spark->rotAdd = Random::GenerateInt(-16, 16);

				spark->sSize = spark->size = Random::GenerateInt(0, 5) * scale + size * 2;
				spark->dSize = (3 * spark->size);

				spark->spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex + (Random::GenerateInt(0, 100) > 70 ? 34 : 0);
				spark->flags = SP_SCALE | SP_DEF | SP_ROTATE;

				if (sign == 1)
				{
					currentStep++;
					if (currentStep == 1)
						break;
				}
			}
		}
	}

	/*void SpawnUnderwaterBloodCloud(const Vector3& pos, int roomNumber, float sizeMax, unsigned int count)
	{
		if (!TestEnvironment(ENV_FLAG_WATER, roomNumber))
			return;

		for (int i = 0; i < count; i++)
			SpawnUnderwaterBlood(pos, roomNumber, sizeMax);
	}

	void UpdateUnderwaterBloodParticles()
	{
		constexpr auto UW_BLOOD_SIZE_MAX = BLOCK(0.25f);

		if (UnderwaterBloodParticles.empty())
			return;

		for (auto& uwBlood : UnderwaterBloodParticles)
		{
			if (uwBlood.Life <= 0.0f)
				continue;

			// Update size.
			if (uwBlood.Size < UW_BLOOD_SIZE_MAX)
				uwBlood.Size += 4.0f;

			// Update life.
			if (uwBlood.Init == 0.0f)
			{
				uwBlood.Life -= 3.0f;
			}
			else if (uwBlood.Init < uwBlood.Life)
			{
				uwBlood.Init += 4.0f;

				if (uwBlood.Init >= uwBlood.Life)
					uwBlood.Init = 0.0f;
			}
		}

		ClearInactiveEffects(UnderwaterBloodParticles);
	}

	void ClearUnderwaterBloodParticles()
	{
		UnderwaterBloodParticles.clear();
	}*/

