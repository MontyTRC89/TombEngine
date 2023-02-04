#include "framework.h"
#include "Game/Hud/PickupSummary.h"

#include "Game/effects/effects.h"
#include "Game/pickup/pickup.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"
#include "Specific/clock.h"
#include "Specific/setup.h"

using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

namespace TEN::Hud
{
	constexpr auto DISPLAY_PICKUP_COUNT_MAX = 64;

	bool DisplayPickup::IsOffscreen(bool checkAbove) const
	{
		auto screenRes = g_Renderer.GetScreenResolution();
		auto threshold = screenRes.ToVector2() * 0.1f;

		// NOTE: Positions above screen can be ignored to account for high stacks.
		if (checkAbove)
		{
			if (Position.y <= -threshold.y)
				return true;
		}

		return (Position.x <= -threshold.x ||
				Position.x >= (screenRes.x + threshold.x) ||
				Position.y >= (screenRes.y + threshold.y));
	}

	void DisplayPickup::Update(bool isHead)
	{
		constexpr auto LIFE_BUFFER		   = 0.2f;
		constexpr auto SCALE_MAX		   = 0.4f;
		constexpr auto SCALE_MIN		   = 0.2f;
		constexpr auto HIDE_VELOCITY_COEFF = 3 / 100.0f;
		constexpr auto POSITION_LERP_ALPHA = 0.2f;
		constexpr auto STRING_SCALAR_ALPHA = 0.75f;
		const	  auto ROTATION			   = EulerAngles(0, ANGLE(3.0f), 0);

		if (this->IsOffscreen(false))
			return;

		// Update position, scale, and opacity.
		if (Life > 0.0f)
		{
			float totalDist = Vector2::Distance(Origin, Target);
			float coveredDist = Vector2::Distance(Origin, Position);

			// Handle edge case when stack shifts.
			if (coveredDist > totalDist)
			{
				this->Origin = Position;
				totalDist = Vector2::Distance(Origin, Target);
				coveredDist = Vector2::Distance(Origin, Position);
			}

			float alpha = coveredDist / totalDist;

			this->Position = Vector2::Lerp(Position, Target, POSITION_LERP_ALPHA);
			this->Scale = std::max(Lerp(SCALE_MIN, SCALE_MAX, alpha), Scale);
			this->Opacity = std::max(Lerp(0.0f, 1.0f, alpha), Opacity);
		}

		// Move offscreen.
		if (isHead && Life <= 0.0f)
		{
			auto screenRes = g_Renderer.GetScreenResolution().ToVector2();
			auto vel = Vector2(screenRes.x * HIDE_VELOCITY_COEFF, 0.0f);
			this->Position += vel;
		}

		// Update orientation.
		this->Orientation += ROTATION;

		// Update string scale.
		float alpha = Scale / SCALE_MAX;
		this->StringScale = Lerp(0.0f, 1.0f, alpha) * (1.0f + StringScalar);
		this->StringScalar *= STRING_SCALAR_ALPHA;

		// Update life.
		this->Life -= 1.0f;
		if (!isHead)
			this->Life = std::max(this->Life, round(LIFE_BUFFER * FPS));
	}

	void PickupSummaryController::AddDisplayPickup(GAME_OBJECT_ID objectID, const Vector3& pos)
	{
		constexpr auto LIFE_MAX			 = 2.5f;
		constexpr auto STRING_SCALAR_MAX = 0.6f;

		// TODO: Call this elsewhere. Maybe add PickUpObject() function to pickup.cpp.
		PickedUpObject(objectID);

		// Display pickup of same type exists; increment its count.
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
		auto& pickup = GetNewDisplayPickup();

		pickup.ObjectID = objectID;
		pickup.Count = 1;
		pickup.Position =
		pickup.Origin = g_Renderer.GetScreenSpacePosition(pos);
		pickup.Target = Vector2::Zero;
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
		auto stackPositions = this->GetStackPositions();
		for (int i = 0; i < stackPositions.size(); i++)
			this->DisplayPickups[i].Target = stackPositions[i];

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

			g_Renderer.DrawPickup(pickup);
		}
	}

	void PickupSummaryController::Clear()
	{
		this->DisplayPickups.clear();
	}

	std::vector<Vector2> PickupSummaryController::GetStackPositions() const
	{
		constexpr auto STACK_HEIGHT_MAX = 6;
		constexpr auto SCALE_COEFF		= 1 / 7.0f;
		constexpr auto OFFSET_COEFF		= 1 / 7.0f;

		// Determine screen values.
		auto screenRes = g_Renderer.GetScreenResolution().ToVector2();
		auto scale = screenRes * SCALE_COEFF;
		auto offset = -(screenRes * OFFSET_COEFF);

		// Calculate screen positions. 
		auto stackPositions = std::vector<Vector2>{};
		for (int i = 0; i < DisplayPickups.size(); i++)
		{
			auto relPos = (i < STACK_HEIGHT_MAX) ? (Vector2(0.0f, i) * scale) : Vector2(0.0f, screenRes.y);
			auto pos = (screenRes - relPos) + offset;
			stackPositions.push_back(pos);
		}

		return stackPositions;
	}

	DisplayPickup& PickupSummaryController::GetNewDisplayPickup()
	{
		// Add and return new display pickup.
		if (DisplayPickups.size() < DISPLAY_PICKUP_COUNT_MAX)
			return this->DisplayPickups.emplace_back();

		// Clear and return most recent display pickup.
		auto& pickup = this->DisplayPickups.back();
		pickup = DisplayPickup();
		return pickup;
	}

	void PickupSummaryController::ClearInactiveDisplayPickups()
	{
		DisplayPickups.erase(
			std::remove_if(
				DisplayPickups.begin(), DisplayPickups.end(),
				[](const DisplayPickup& pickup) { return ((pickup.Life <= 0.0f) && pickup.IsOffscreen(false)); }),
			DisplayPickups.end());
	}

	void PickupSummaryController::DrawDebug() const
	{
		g_Renderer.PrintDebugMessage("Num. display pickups: %d", DisplayPickups.size());
	}
}
