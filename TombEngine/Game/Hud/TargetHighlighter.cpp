#include "framework.h"
#include "Game/Hud/TargetHighlighter.h"

#include "Game/camera.h"
#include "Game/items.h"
#include "Game/lara/lara_fire.h"
#include "Game/lara/lara_helpers.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"

using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

namespace TEN::Hud
{
	static float GetCrosshairSize(float dist)
	{
		constexpr auto RANGE			  = BLOCK(10);
		constexpr auto CROSSHAIR_SIZE_MAX = SCREEN_SPACE_RES.y * 0.25f;
		constexpr auto CROSSHAIR_SIZE_MIN = CROSSHAIR_SIZE_MAX / 5;

		auto distAlpha = dist / RANGE;
		return Lerp(CROSSHAIR_SIZE_MAX, CROSSHAIR_SIZE_MIN, distAlpha);
	}

	bool CrosshairData::IsOffscreen() const
	{
		float screenEdgeThreshold = ((Size * 2) * (RadiusScalar + 1.0f)) * SQRT_2; // TODO: Check.

		return (Position2D.x <= -screenEdgeThreshold ||
				Position2D.y <= -screenEdgeThreshold ||
				Position2D.x >= (SCREEN_SPACE_RES.x + screenEdgeThreshold) ||
				Position2D.y >= (SCREEN_SPACE_RES.y + screenEdgeThreshold));
	}

	void CrosshairData::Update(const Vector3& cameraPos, bool isActive)
	{
		constexpr auto INVALID_2D_POS		= Vector2(FLT_MAX);
		constexpr auto ROT					= ANGLE(2.0f);
		constexpr auto RADIUS_ALPHA_PRIMARY = 0.5f;
		constexpr auto MORPH_LERP_ALPHA		= 0.3f;
		constexpr auto ORIENT_LERP_ALPHA	= 0.1f;
		constexpr auto RADIUS_LERP_ALPHA	= 0.2f;

		// Update active status.
		IsActive = isActive;

		// Update 2D position.
		auto pos2D = g_Renderer.Get2DPosition(cameraPos);
		Position2D = pos2D.has_value() ? pos2D.value() : INVALID_2D_POS;

		// Update 2D orientation.
		if (IsPrimary)
		{
			Orientation2D += ROT;
		}
		else
		{
			// TODO: Inaccurate.
			short closestCardinalAngle = (Orientation2D / ANGLE(90.0f)) * ANGLE(90.0f);
			Orientation2D = (short)round(Lerp(Orientation2D, closestCardinalAngle, ORIENT_LERP_ALPHA));
		}

		// Update color.
		ColorTarget.w = IsActive ? ColorTarget.w : 0.0f;
		Color = Vector4::Lerp(Color, ColorTarget, MORPH_LERP_ALPHA);

		// Update size.
		if (IsActive)
		{
			float dist = Vector3::Distance(Camera.pos.ToVector3(), cameraPos);
			float sizeTarget = GetCrosshairSize(dist);
			Size = Lerp(Size, sizeTarget, MORPH_LERP_ALPHA);
		}
		else
		{
			Size = Lerp(Size, 0.0f, MORPH_LERP_ALPHA);
		}

		// Update radius scalar.
		RadiusScalarTarget = IsActive ? (IsPrimary ? RADIUS_ALPHA_PRIMARY : 0.0f) : RADIUS_SCALAR_MAX;
		RadiusScalar = Lerp(RadiusScalar, RadiusScalarTarget, RADIUS_LERP_ALPHA);
	}

	void TargetHighlighterController::SetPrimary(std::vector<int> entityIds)
	{
		// No crosshairs to set; return early.
		if (Crosshairs.empty() || entityIds.empty())
			return;

		// Set crosshairs as primary.
		for (int entityID : entityIds)
		{
			// Matching crosshair; continue.
			if (!Crosshairs.count(entityID))
				continue;

			auto& crosshair = Crosshairs.at(entityID);

			crosshair.IsPrimary = true;
			crosshair.ColorTarget = CrosshairData::COLOR_GREEN;
		}
	}

	void TargetHighlighterController::SetPrimary(int entityID)
	{
		SetPrimary(std::vector<int>{ entityID });
	}

	void TargetHighlighterController::SetPeripheral(std::vector<int> entityIds)
	{
		// No crosshairs to set; return early.
		if (Crosshairs.empty() || entityIds.empty())
			return;

		// Set crosshairs as peripheral.
		for (int entityID : entityIds)
		{
			// Matching crosshair; continue.
			if (!Crosshairs.count(entityID))
				continue;

			auto& crosshair = Crosshairs.at(entityID);

			crosshair.IsPrimary = false;
			crosshair.ColorTarget = CrosshairData::COLOR_GRAY;
		}
	}

	void TargetHighlighterController::SetPeripheral(int entityID)
	{
		SetPeripheral(std::vector<int>{ entityID });
	}

	void TargetHighlighterController::Update(std::vector<int> entityIds)
	{
		// No crosshairs to update; return early.
		if (Crosshairs.empty() && entityIds.empty())
			return;

		// Update active crosshairs.
		for (int entityID : entityIds)
		{
			const auto& item = g_Level.Items[entityID];
			auto pos = GetJointPosition(item, 0).ToVector3();

			// Find crosshair.
			auto it = Crosshairs.find(entityID);

			// Update existing active crosshair.
			if (it != Crosshairs.end() && it->second.IsActive)
			{
				auto& crosshair = it->second;
				crosshair.Update(pos, true);
			}
			// Add new active crosshair.
			else
			{
				AddTargetHighlight(entityID, pos);
			}
		}

		// Update inactive crosshairs.
		for (auto& [entityID, crosshair] : Crosshairs)
		{
			// Find absent crosshairs.
			auto it = std::find(entityIds.begin(), entityIds.end(), entityID);
			if (it != entityIds.end())
				continue;

			// Get position.
			const auto& item = g_Level.Items[entityID];
			auto pos = GetJointPosition(item, 0).ToVector3();

			// Update inactive crosshair.
			crosshair.Update(pos, false);
		}

		ClearInactiveTargetHighlights();
	}

	void TargetHighlighterController::Update(const ItemInfo& playerItem)
	{
		const auto& player = GetLaraInfo(playerItem);

		auto entityIds = GetTargetEntityIds(playerItem);

		// Determine peripheral entity IDs.
		auto peripheralEntityIds = entityIds;
		if (player.TargetEntity != nullptr)
		{
			// Set primary crosshair.
			int primaryEntityID = player.TargetEntity->Index;
			SetPrimary(primaryEntityID);

			// Clear primary target entity ID from peripheral entity IDs.
			auto it = std::find(peripheralEntityIds.begin(), peripheralEntityIds.end(), primaryEntityID);
			if (it != peripheralEntityIds.end())
				peripheralEntityIds.erase(it);
		}

		// Set peripheral crosshairs.
		SetPeripheral(peripheralEntityIds);

		// Update crosshairs.
		Update(entityIds);
	}

	void TargetHighlighterController::Draw() const
	{
		DrawDebug();

		if (Crosshairs.empty())
			return;

		for (const auto& [entityID, crosshair] : Crosshairs)
		{
			if (crosshair.IsOffscreen())
				continue;

			g_Renderer.DrawSpriteIn2DSpace(
				(GAME_OBJECT_ID)1380, 0,
				crosshair.Position2D, crosshair.Orientation2D, crosshair.Color, Vector2(crosshair.Size));
		}
	}

	void TargetHighlighterController::Clear()
	{
		Crosshairs.clear();
	}

	std::vector<int> TargetHighlighterController::GetTargetEntityIds(const ItemInfo& playerItem)
	{
		if (!playerItem.IsLara())
			return {};

		auto& player = GetLaraInfo(playerItem);

		auto entityIds = std::vector<int>{};
		for (const auto* targetPtr : player.TargetList)
		{
			if (targetPtr == nullptr)
				continue;

			entityIds.push_back((int)targetPtr->Index);
		}

		return entityIds;
	}

	CrosshairData& TargetHighlighterController::GetNewTargetHighlight(int entityID)
	{
		constexpr auto COUNT_MAX = 16;

		// Clear smallest crosshair if map is full.
		if (Crosshairs.size() >= COUNT_MAX)
		{
			int key = 0;
			float smallestSize = INFINITY;
			
			for (auto& [entityID, crosshair] : Crosshairs)
			{
				if (crosshair.Size < smallestSize)
				{
					key = entityID;
					smallestSize = crosshair.Size;
				}
			}

			Crosshairs.erase(key);
		}

		// Return new crosshair.
		Crosshairs.insert({ entityID, {} });
		auto& crosshair = Crosshairs.at(entityID);
		crosshair = {};
		return crosshair;
	}

	void TargetHighlighterController::AddTargetHighlight(int entityID, const Vector3& pos)
	{
		constexpr auto SIZE_DEFAULT = SCREEN_SPACE_RES.x / 2;

		auto pos2D = g_Renderer.Get2DPosition(pos);
		if (!pos2D.has_value())
			return;

		// Create new crosshair.
		auto& crosshair = GetNewTargetHighlight(entityID);

		crosshair.IsActive = true;
		crosshair.IsPrimary = false;
		crosshair.Position2D = pos2D.value();
		crosshair.Orientation2D = 0;
		crosshair.Color = CrosshairData::COLOR_GRAY;
		crosshair.Color.w = 0.0f;
		crosshair.ColorTarget = CrosshairData::COLOR_GRAY;
		crosshair.Size = SIZE_DEFAULT;
		crosshair.RadiusScalar = CrosshairData::RADIUS_SCALAR_MAX;
		crosshair.RadiusScalarTarget = 0.0f;
	}

	void TargetHighlighterController::ClearInactiveTargetHighlights()
	{
		for (auto it = Crosshairs.begin(); it != Crosshairs.end();)
		{
			// Clear crosshair if inactive and size is near 0.
			if (!it->second.IsActive && (it->second.Size <= EPSILON))
			{
				it = Crosshairs.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	void TargetHighlighterController::DrawDebug() const
	{
		unsigned int primaryCount = 0;
		unsigned int peripheralCount = 0;

		for (const auto& [entityID, crosshair] : Crosshairs)
			crosshair.IsPrimary ? primaryCount++ : peripheralCount++;

		g_Renderer.PrintDebugMessage("Highlights: %d", Crosshairs.size());
		g_Renderer.PrintDebugMessage("Primary crosshairs: %d", primaryCount);
		g_Renderer.PrintDebugMessage("Peripheral crosshairs: %d", peripheralCount);
	}
}
