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

		return (Position2D.x <= -SCREEN_THRESHOLD.x ||
				Position2D.y <= -SCREEN_THRESHOLD.y ||
				Position2D.x >= (SCREEN_SPACE_RES.x + SCREEN_THRESHOLD.x) ||
				Position2D.y >= (SCREEN_SPACE_RES.y + SCREEN_THRESHOLD.y));
	}

	void DisplayPickup::Update(bool isHead)
	{
		constexpr auto LIFE_BUFFER		   = 0.2f;
		constexpr auto SCALE_MAX		   = 0.4f;
		constexpr auto SCALE_MIN		   = 0.25f;
		constexpr auto HIDE_VELOCITY_MAX   = SCREEN_SPACE_RES.x * 0.03f;
		constexpr auto HIDE_VELOCITY_ACCEL = HIDE_VELOCITY_MAX / 4;
		constexpr auto POSITION_LERP_ALPHA = 0.2f;
		constexpr auto STRING_SCALAR_ALPHA = 0.25f;
		constexpr auto ROTATION			   = EulerAngles(0, ANGLE(3.0f), 0);

		// Move offscreen.
		if (Life <= 0.0f && isHead)
		{
			this->HideVelocity = std::clamp(HideVelocity + HIDE_VELOCITY_ACCEL, 0.0f, HIDE_VELOCITY_MAX);
			this->Position2D.x += HideVelocity;
		}
		// Update position, scale, and opacity.
		else if (Life > 0.0f)
		{
			float totalDist = Vector2::Distance(Origin2D, Target2D);
			float coveredDist = Vector2::Distance(Origin2D, Position2D);

			// Handle edge case when stack shifts.
			if (coveredDist > totalDist)
			{
				this->Origin2D = Position2D;
				totalDist = Vector2::Distance(Origin2D, Target2D);
				coveredDist = Vector2::Distance(Origin2D, Position2D);
			}

			float alpha = coveredDist / totalDist;

			this->Position2D = Vector2::Lerp(Position2D, Target2D, POSITION_LERP_ALPHA);
			this->Scale = std::max(Lerp(SCALE_MIN, SCALE_MAX, alpha), Scale);
			this->Opacity = std::max(Lerp(0.0f, 1.0f, alpha), Opacity);
		}

		// Update orientation.
		this->Orientation += ROTATION;

		// Update string scale.
		float alpha = Scale / SCALE_MAX;
		this->StringScale = Lerp(0.0f, 1.0f, alpha) * (1.0f + StringScalar);
		this->StringScalar = Lerp(StringScalar, 0.0f, STRING_SCALAR_ALPHA);

		// Update life.
		this->Life -= 1.0f;
		if (!isHead)
			this->Life = std::max(Life, round(LIFE_BUFFER * FPS));
	}

	void PickupSummaryController::AddDisplayPickup(GAME_OBJECT_ID objectID, const Vector3& pos)
	{
		constexpr auto LIFE_MAX			 = 2.5f;
		constexpr auto STRING_SCALAR_MAX = 0.6f;

		// TODO: Call this elsewhere, maybe in pickup.cpp. -- Sezz 2023.02.06
		PickedUpObject(objectID);

		// Increment count of existing display pickup if it exists.
		for (auto& pickup : this->DisplayPickups)
		{
			// Ignore already disappearing display pickups.
			if (pickup.Life <= 0.0f)
				continue;

			if (pickup.ObjectID == objectID)
			{
				pickup.Count++;
				pickup.Life = round(LIFE_MAX * FPS);
				pickup.StringScalar = STRING_SCALAR_MAX;
				return;
			}
		}

		// Create new display pickup.
		auto& pickup = this->GetNewDisplayPickup();

		auto origin2D = g_Renderer.Get2DPosition(pos);
		if (origin2D == INVALID_2D_POSITION)
			origin2D = Vector2::Zero;

		pickup.ObjectID = objectID;
		pickup.Count = 1;
		pickup.Position2D =
		pickup.Origin2D = origin2D;
		pickup.Target2D = Vector2::Zero;
		pickup.Life = round(LIFE_MAX * FPS);
		pickup.Scale = 0.0f;
		pickup.Opacity = 0.0f;
		pickup.HideVelocity = 0.0f;
		pickup.StringScale = 0.0f;
		pickup.StringScalar = 0.0f;
	}

	void PickupSummaryController::Update()
	{
		if (DisplayPickups.empty())
			return;

		// Get and apply stack screen positions as targets.
		auto stack2DPositions = this->GetStack2DPositions();
		for (int i = 0; i < stack2DPositions.size(); i++)
			this->DisplayPickups[i].Target2D = stack2DPositions[i];

		// Update display pickups.
		bool isHead = true;
		for (auto& pickup : this->DisplayPickups)
		{
			pickup.Update(isHead);
			isHead = false;
		}

		this->ClearInactiveDisplayPickups();
	}

	void PickupSummaryController::Draw() const
	{
		//this->DrawDebug();

		if (DisplayPickups.empty())
			return;

		// Draw display pickups.
		for (const auto& pickup : this->DisplayPickups)
		{
			if (pickup.IsOffscreen())
				continue;

			g_Renderer.DrawDisplayPickup(pickup);
		}
	}

	void PickupSummaryController::Clear()
	{
		this->DisplayPickups.clear();
	}

	std::vector<Vector2> PickupSummaryController::GetStack2DPositions() const
	{
		constexpr auto STACK_HEIGHT_MAX	   = 6;
		constexpr auto SCREEN_SCALE_COEFF  = 1 / 7.0f;
		constexpr auto SCREEN_OFFSET_COEFF = 1 / 7.0f;
		constexpr auto SCREEN_SCALE		   = Vector2(SCREEN_SPACE_RES.x * SCREEN_SCALE_COEFF, SCREEN_SPACE_RES.y * SCREEN_SCALE_COEFF);
		constexpr auto SCREEN_OFFSET	   = Vector2(SCREEN_SPACE_RES.y * SCREEN_OFFSET_COEFF);

		// Calculate 2D positions. 
		auto stack2DPositions = std::vector<Vector2>{};
		for (int i = 0; i < DisplayPickups.size(); i++)
		{
			auto relPos = (i < STACK_HEIGHT_MAX) ? (Vector2(0.0f, i) * SCREEN_SCALE) : Vector2(0.0f, SCREEN_SPACE_RES.y);
			auto pos = (SCREEN_SPACE_RES - relPos) - SCREEN_OFFSET;
			stack2DPositions.push_back(pos);
		}

		return stack2DPositions;
	}

	DisplayPickup& PickupSummaryController::GetNewDisplayPickup()
	{
		// Add and return new display pickup.
		if (DisplayPickups.size() < DISPLAY_PICKUP_COUNT_MAX)
			return this->DisplayPickups.emplace_back();

		// Clear and return most recent display pickup.
		auto& pickup = this->DisplayPickups.back();
		pickup = {};
		return pickup;
	}

	void PickupSummaryController::ClearInactiveDisplayPickups()
	{
		this->DisplayPickups.erase(
			std::remove_if(
				this->DisplayPickups.begin(), this->DisplayPickups.end(),
				[](const DisplayPickup& pickup) { return ((pickup.Life <= 0.0f) && pickup.IsOffscreen()); }),
			this->DisplayPickups.end());
	}

	void PickupSummaryController::DrawDebug() const
	{
		g_Renderer.PrintDebugMessage("Num. display pickups: %d", DisplayPickups.size());
	}
}
