#include "framework.h"
#include "Objects/Effects/LensFlare.h"

#include "Game/camera.h"
#include "Game/control/los.h"
#include "Math/Math.h"
#include "Scripting/Include/ScriptInterfaceLevel.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/level.h"

using namespace TEN::Math;

namespace TEN::Entities::Effects
{
	constexpr float MAX_INTENSITY = 1.0f;		// Maximum intensity
	constexpr float FADE_SPEED = 0.10f;			// Speed of fade-in/out per frame
	constexpr float SHIMMER_STRENGTH = 0.15f;	// Max shimmer amplitude

	float GlobalLensFlareIntensity = 0;

	std::vector<LensFlare> LensFlares;
	
	static void UpdateLensFlareIntensity(bool isVisible, float& intensity)
	{
		// Target intensity based on visibility.
		float targetIntensity = isVisible ? MAX_INTENSITY : 0.0f;

		// Fade-in or fade-out.
		if (intensity < targetIntensity)
			intensity = std::min(intensity + FADE_SPEED, targetIntensity);
		else if (intensity > targetIntensity)
			intensity = std::max(intensity - FADE_SPEED, targetIntensity);
	}

	static void AdjustLensflarePosition(Vector3& lensFlarePos, const Vector3& targetPos, float divisor, float threshold)
	{
		// Gradually narrow lensflare vector down to target vector, until threshold is met.
		auto delta = Vector3(FLT_MAX);
		while (delta.Length() > threshold)
		{
			delta = lensFlarePos - targetPos;
			lensFlarePos -= delta / divisor;
		}
	}

	static void SetupLensFlare(const Vector3& pos, int roomNumber, const Color& color, float& intensity, int spriteID)
	{
		auto cameraPos = Camera.pos.ToVector3();
		auto cameraTarget = Camera.target.ToVector3();

		auto forward = (cameraTarget - cameraPos);
		forward.Normalize();

		// Discard lensflares behind camera.
		if (forward.Dot(pos - cameraPos) < 0.0f)
			return;

		auto lensFlarePos = pos;
		bool isGlobal = roomNumber == NO_VALUE;

		if (isGlobal)
		{
			if (TestEnvironment(ENV_FLAG_NO_LENSFLARE, Camera.pos.RoomNumber))
				return;

			AdjustLensflarePosition(lensFlarePos, cameraPos, 8.0f, BLOCK(256));
			AdjustLensflarePosition(lensFlarePos, cameraPos, 4.0f, BLOCK(32));

			// FAILSAFE: Break while loop if room can't be found (e.g. camera is in the void).
			int narrowingCycleCount = 0;

			while (roomNumber == NO_VALUE && narrowingCycleCount < 16)
			{
				int foundRoomNumber = IsRoomOutside(lensFlarePos.x, lensFlarePos.y, lensFlarePos.z);
				if (foundRoomNumber != NO_VALUE)
				{
					roomNumber = foundRoomNumber;
					break;
				}

				AdjustLensflarePosition(lensFlarePos, cameraPos, 2.0f, BLOCK(32));
				narrowingCycleCount++;
			}
		}
		else
		{
			float dist = Vector3::Distance(lensFlarePos, cameraPos);
			if (dist > BLOCK(32))
				return;
		}

		bool isVisible = false;
		if (roomNumber != NO_VALUE)
		{
			if (TestEnvironment(ENV_FLAG_NOT_NEAR_OUTSIDE, roomNumber) || !isGlobal)
			{
				auto origin = GameVector(lensFlarePos, roomNumber);
				auto target = Camera.pos;

				MESH_INFO* mesh = nullptr;
				auto tempVector = Vector3i();

				isVisible = LOS(&origin, &target) && ObjectOnLOS2(&origin, &target, &tempVector, &mesh) == NO_LOS_ITEM && 
							ObjectOnLOS2(&origin, &target, &tempVector, nullptr, ID_LARA) == NO_LOS_ITEM;
			}
		}

		UpdateLensFlareIntensity(isVisible, intensity);

		if (!isVisible && !isGlobal && intensity == 0.0f)
			return;

		float shimmer = Random::GenerateFloat(-SHIMMER_STRENGTH, SHIMMER_STRENGTH);
		float finalIntensity = std::clamp(Smoothstep(intensity) + shimmer * intensity, 0.0f, MAX_INTENSITY);

		auto lensFlare = LensFlare{};
		lensFlare.Position = pos;
		lensFlare.RoomNumber = roomNumber;
		lensFlare.IsGlobal = isGlobal;
		lensFlare.Color = color * Smoothstep(finalIntensity);
		lensFlare.SpriteID = spriteID;

		LensFlares.push_back(lensFlare);
	}

	void UpdateGlobalLensFlare()
	{
		constexpr auto BASE_POS = Vector3(0.0f, 0.0f, BLOCK(256));

		if (!g_GameFlow->GetLevel(CurrentLevel)->GetLensFlareEnabled())
			return;

		auto orient   = EulerAngles(g_GameFlow->GetLevel(CurrentLevel)->GetLensFlarePitch(), g_GameFlow->GetLevel(CurrentLevel)->GetLensFlareYaw(), 0);
		auto color    = g_GameFlow->GetLevel(CurrentLevel)->GetLensFlareColor();
		auto spriteID = g_GameFlow->GetLevel(CurrentLevel)->GetLensFlareSunSpriteID();
		
		auto pos = Camera.pos.ToVector3();
		auto rotMatrix = orient.ToRotationMatrix();

		pos += Vector3::Transform(BASE_POS, rotMatrix);
		SetupLensFlare(pos, NO_VALUE, color, GlobalLensFlareIntensity, spriteID);
	}

	void ControlLensFlare(int itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (TriggerActive(&item))
		{
			float currentIntensity = (float)item.ItemFlags[0] / 100.0f;
			SetupLensFlare(item.Pose.Position.ToVector3(), item.RoomNumber, item.Model.Color, currentIntensity, SPRITE_TYPES::SPR_LENS_FLARE_3);
			item.ItemFlags[0] = (short)(currentIntensity * 100.0f);
		}
	}

	void ClearLensFlares()
	{
		LensFlares.clear();
	}
}
