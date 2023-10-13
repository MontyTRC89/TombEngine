#include "framework.h"
#include "Game/Hud/PickupSummary.h"

#include "Game/pickup/pickup.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"
#include "Specific/clock.h"

using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

namespace TEN::Hud
{
	bool DisplayPickup::IsOffscreen() const
	{
		constexpr auto SCREEN_THRESHOLD_COEFF = 0.1f;
		constexpr auto SCREEN_THRESHOLD		  = Vector2(SCREEN_SPACE_RES.x * SCREEN_THRESHOLD_COEFF, SCREEN_SPACE_RES.y * SCREEN_THRESHOLD_COEFF);

		if (Position.x <= -SCREEN_THRESHOLD.x ||
			Position.y <= -SCREEN_THRESHOLD.y ||
			Position.x >= (SCREEN_SPACE_RES.x + SCREEN_THRESHOLD.x) ||
			Position.y >= (SCREEN_SPACE_RES.y + SCREEN_THRESHOLD.y))
		{
			return true;
		}

		return false;
	}

	void DisplayPickup::Update(bool isHead)
	{
		constexpr auto LIFE_BUFFER		   = 0.2f;
		constexpr auto SCALE_MAX		   = 0.4f;
		constexpr auto SCALE_MIN		   = 0.25f;
		constexpr auto HIDE_VEL_MAX		   = SCREEN_SPACE_RES.x * 0.03f;
		constexpr auto HIDE_VEL_ACCEL	   = HIDE_VEL_MAX / 4;
		constexpr auto POS_LERP_ALPHA	   = 0.2f;
		constexpr auto STRING_SCALAR_ALPHA = 0.25f;
		constexpr auto ROT_RATE			   = ANGLE(360.0f / (LIFE_MAX * FPS));
		constexpr auto ROT				   = EulerAngles(0, ROT_RATE, 0);

		// Move offscreen.
		if (Life <= 0.0f && isHead)
		{
			HideVelocity = std::clamp(HideVelocity + HIDE_VEL_ACCEL, 0.0f, HIDE_VEL_MAX);
			Position.x += HideVelocity;
		}
		// Update position, scale, and opacity.
		else if (Life > 0.0f)
		{
			float totalDist = Vector2::Distance(Origin, Target);
			float coveredDist = Vector2::Distance(Origin, Position);

			// Handle edge case when stack shifts.
			if (coveredDist > totalDist)
			{
				Origin = Position;
				totalDist = Vector2::Distance(Origin, Target);
				coveredDist = Vector2::Distance(Origin, Position);
			}

			float alpha = coveredDist / totalDist;

			Position = Vector2::Lerp(Position, Target, POS_LERP_ALPHA);
			Scale = std::max(Lerp(SCALE_MIN, SCALE_MAX, alpha), Scale);
			Opacity = std::max(Lerp(0.0f, 1.0f, alpha), Opacity);
		}

		// Update orientation.
		Orientation += ROT;

		// Update string scale.
		float alpha = Scale / SCALE_MAX;
		StringScale = Lerp(0.0f, 1.0f, alpha) * (1.0f + StringScalar);
		StringScalar = Lerp(StringScalar, 0.0f, STRING_SCALAR_ALPHA);

		// Update life.
		Life -= 1.0f;
		if (!isHead)
			Life = std::max(Life, round(LIFE_BUFFER * FPS));
	}

	void PickupSummaryController::AddDisplayPickup(GAME_OBJECT_ID objectID, const Vector2& origin, unsigned int count)
	{
		constexpr auto STRING_SCALAR_MAX = 0.6f;

		// No count; return early.
		if (count == 0)
			return;

		float life = round(DisplayPickup::LIFE_MAX * FPS);

		// Increment count of existing display pickup if it exists.
		for (auto& pickup : DisplayPickups)
		{
			// Ignore already disappearing display pickups.
			if (pickup.Life <= 0.0f)
				continue;

			if (pickup.ObjectID == objectID)
			{
				pickup.Count += count;
				pickup.Life = life;
				pickup.StringScalar = STRING_SCALAR_MAX;
				return;
			}
		}

		// Create new display pickup.
		auto& pickup = GetNewDisplayPickup();

		pickup.ObjectID = objectID;
		pickup.Count = count;
		pickup.Position =
		pickup.Origin = origin;
		pickup.Target = Vector2::Zero;
		pickup.Life = life;
		pickup.Scale = 0.0f;
		pickup.Opacity = 0.0f;
		pickup.HideVelocity = 0.0f;
		pickup.StringScale = 0.0f;
		pickup.StringScalar = 0.0f;
	}

	void PickupSummaryController::AddDisplayPickup(GAME_OBJECT_ID objectID, const Vector3& pos, unsigned int count)
	{
		// Project 3D position to 2D origin.
		auto origin = g_Renderer.Get2DPosition(pos);

		AddDisplayPickup(objectID, origin.value_or(Vector2::Zero), count);
	}

	void PickupSummaryController::Update()
	{
		if (DisplayPickups.empty())
			return;

		// Get and apply stack positions as targets.
		auto stackPositions = GetStackPositions();
		for (int i = 0; i < stackPositions.size(); i++)
			DisplayPickups[i].Target = std::move(stackPositions[i]);

		// Update display pickups.
		bool isHead = true;
		for (auto& pickup : DisplayPickups)
		{
			pickup.Update(isHead);
			isHead = false;
		}

		ClearInactiveDisplayPickups();
	}

	void PickupSummaryController::Draw() const
	{
		//DrawDebug();

		if (DisplayPickups.empty())
			return;

		// Draw display pickups.
		for (const auto& pickup : DisplayPickups)
		{
			if (pickup.IsOffscreen())
				continue;

			g_Renderer.DrawDisplayPickup(pickup);
		}
	}

	void PickupSummaryController::Clear()
	{
		DisplayPickups.clear();
	}

	std::vector<Vector2> PickupSummaryController::GetStackPositions() const
	{
		constexpr auto STACK_HEIGHT_MAX	   = 6;
		constexpr auto SCREEN_SCALE_COEFF  = 1 / 7.0f;
		constexpr auto SCREEN_OFFSET_COEFF = 1 / 7.0f;
		constexpr auto SCREEN_SCALE		   = Vector2(SCREEN_SPACE_RES.x * SCREEN_SCALE_COEFF, SCREEN_SPACE_RES.y * SCREEN_SCALE_COEFF);
		constexpr auto SCREEN_OFFSET	   = Vector2(SCREEN_SPACE_RES.y * SCREEN_OFFSET_COEFF);

		// Calculate stack positions. 
		auto stackPositions = std::vector<Vector2>{};
		stackPositions.resize(DisplayPickups.size());
		for (int i = 0; i < DisplayPickups.size(); i++)
		{
			auto relPos = (i < STACK_HEIGHT_MAX) ? (Vector2(0.0f, i) * SCREEN_SCALE) : Vector2(0.0f, SCREEN_SPACE_RES.y);
			auto pos = (SCREEN_SPACE_RES - relPos) - SCREEN_OFFSET;
			stackPositions[i] = pos;
		}

		return stackPositions;
	}

	DisplayPickup& PickupSummaryController::GetNewDisplayPickup()
	{
		assertion(DisplayPickups.size() <= DISPLAY_PICKUP_COUNT_MAX, "Display pickup overflow.");

		// Add and return new display pickup.
		if (DisplayPickups.size() < DISPLAY_PICKUP_COUNT_MAX)
			return DisplayPickups.emplace_back();

		// Clear and return most recent display pickup.
		auto& pickup = DisplayPickups.back();
		pickup = {};
		return pickup;
	}

	void PickupSummaryController::ClearInactiveDisplayPickups()
	{
		DisplayPickups.erase(
			std::remove_if(
				DisplayPickups.begin(), DisplayPickups.end(),
				[](const DisplayPickup& pickup) { return ((pickup.Life <= 0.0f) && pickup.IsOffscreen()); }),
			DisplayPickups.end());
	}

	void PickupSummaryController::DrawDebug() const
	{
		g_Renderer.PrintDebugMessage("Display pickups in summary: %d", DisplayPickups.size());
	}
}
