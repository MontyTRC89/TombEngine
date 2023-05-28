#include "framework.h"
#include "Game/Hud/TargetHighlighter.h"

#include "Game/camera.h"
#include "Game/items.h"
#include "Game/lara/lara_helpers.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"

using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

namespace TEN::Hud
{
	static float GetTargetHighlightSize(float dist)
	{
		constexpr auto DIST_MAX					 = BLOCK(10);
		constexpr auto TARGET_HIGHLIGHT_SIZE_MAX = 150.0f;
		constexpr auto TARGET_HIGHLIGHT_SIZE_MIN = TARGET_HIGHLIGHT_SIZE_MAX / 5;

		auto distAlpha = dist / DIST_MAX;
		return Lerp(TARGET_HIGHLIGHT_SIZE_MAX, TARGET_HIGHLIGHT_SIZE_MIN, distAlpha);
	}

	bool TargetHighlightData::IsOffscreen() const
	{
		float screenEdgeThreshold = ((Size * 2) + Radius) * SQRT_2;

		return (Position2D.x <= -screenEdgeThreshold ||
				Position2D.y <= -screenEdgeThreshold ||
				Position2D.x >= (SCREEN_SPACE_RES.x + screenEdgeThreshold) ||
				Position2D.y >= (SCREEN_SPACE_RES.y + screenEdgeThreshold));
	}

	void TargetHighlightData::Update(const Vector3& pos, bool isActive)
	{
		constexpr auto INVALID_2D_POS = Vector2(FLT_MAX); // TODO: Set inactive instead.
		constexpr auto OPACITY_MAX	  = 0.9f;
		constexpr auto ROT			  = ANGLE(2.0f);
		constexpr auto LERP_ALPHA	  = 0.3f;

		// Update active status.
		IsActive = isActive;

		// Update size.
		float sizeTarget = 0.0;
		if (IsActive)
		{
			float dist = Vector3::Distance(Camera.pos.ToVector3(), pos);
			sizeTarget = GetTargetHighlightSize(dist);
		}
		Size = Lerp(Size, sizeTarget, LERP_ALPHA);

		// Update 2D orientation.
		if (IsPrimary)
		{
			Orientation2D += ROT;
		}
		else
		{
			short closestCardinalAngle = (Orientation2D / ANGLE(90.0f)) * ANGLE(90.0f);
			Orientation2D = (short)round(Lerp(Orientation2D, closestCardinalAngle, LERP_ALPHA));
		}

		// Update 2D position.
		auto pos2D = g_Renderer.Get2DPosition(pos);
		Position2D = pos2D.has_value() ? pos2D.value() : INVALID_2D_POS;

		// Update color and opacity.
		OpacityTarget = IsActive ? OpacityTarget : 0.0f;
		ColorTarget.w = OpacityTarget;
		Color = Vector4::Lerp(Color, ColorTarget, LERP_ALPHA);
	}

	void TargetHighlighterController::SetPrimary(std::vector<int> entityIds)
	{
		if (TargetHighlights.empty() || entityIds.empty())
			return;

		// Set entity highlights as primary.
		for (const int& entityID : entityIds)
		{
			// Entity not targeted; continue.
			if (!TargetHighlights.count(entityID))
				continue;

			auto& highlight = TargetHighlights.at(entityID);

			highlight.IsPrimary = true;
			highlight.ColorTarget = TargetHighlightData::COLOR_GREEN;
		}
	}

	void TargetHighlighterController::SetPrimary(int entityID)
	{
		SetPrimary(std::vector<int>{ entityID });
	}

	void TargetHighlighterController::SetPeripheral(std::vector<int> entityIds)
	{
		if (TargetHighlights.empty() || entityIds.empty())
			return;

		// Set entity highlights as peripheral.
		for (const int& entityID : entityIds)
		{
			// Entity not targeted; continue.
			if (!TargetHighlights.count(entityID))
				continue;

			auto& highlight = TargetHighlights.at(entityID);

			highlight.IsPrimary = false;
			highlight.ColorTarget = TargetHighlightData::COLOR_GRAY;
		}
	}

	void TargetHighlighterController::SetPeripheral(int entityID)
	{
		SetPeripheral(std::vector<int>{ entityID });
	}

	void TargetHighlighterController::Update(std::vector<int> entityIds)
	{
		if (TargetHighlights.empty() && entityIds.empty())
			return;

		// Update active highlights.
		for (const int& entityID : entityIds)
		{
			const auto& item = g_Level.Items[entityID];
			auto pos = GetJointPosition(item, 0).ToVector3();

			// Find entity highlight.
			auto it = TargetHighlights.find(entityID);

			// Add new active highlight.
			if (it == TargetHighlights.end())
			{
				AddTargetHighlight(entityID, pos);
			}
			// Update existing active highlight.
			else
			{
				auto& highlight = it->second;
				if (highlight.IsActive)
					highlight.Update(pos, true);
			}
		}

		// Update inactive highlights.
		for (auto& [entityID, highlight] : TargetHighlights)
		{
			// Find absent entity highlights.
			auto it = std::find(entityIds.begin(), entityIds.end(), entityID);
			if (it != entityIds.end())
				continue;

			const auto& item = g_Level.Items[entityID];
			if (item.HitPoints <= 0)
				continue;

			auto pos = item.Pose.Position.ToVector3();

			// Update inactive highlight.
			auto& highlight = TargetHighlights.at(entityID);
			highlight.Update(pos, false);
		}

		ClearInactiveTargetHighlights();
	}

	void TargetHighlighterController::Update(const ItemInfo& playerItem)
	{
		const auto& player = GetLaraInfo(playerItem);

		auto entityIds = GetTargetEntityIds(playerItem);

		// Determine peripheral highlight entity IDs.
		auto peripheralEntityIds = entityIds;
		if (player.TargetEntity != nullptr)
		{
			// Set primary highlight.
			int primaryEntityID = player.TargetEntity->Index;
			SetPrimary(primaryEntityID);

			// Clear primary target entity ID from peripheral entity IDs.
			auto it = std::find(peripheralEntityIds.begin(), peripheralEntityIds.end(), primaryEntityID);
			if (it != peripheralEntityIds.end())
				peripheralEntityIds.erase(it);
		}

		// Set peripheral highlights.
		SetPeripheral(peripheralEntityIds);

		// Update highlights.
		Update(entityIds);
	}

	void TargetHighlighterController::Draw() const
	{
		DrawDebug();

		if (TargetHighlights.empty())
			return;

		for (const auto& [entityID, highlight] : TargetHighlights)
		{
			if (highlight.IsOffscreen())
				continue;

			g_Renderer.DrawSpriteIn2DSpace(
				ID_BINOCULAR_GRAPHIC, 0,
				highlight.Position2D, highlight.Orientation2D, highlight.Color, Vector2(highlight.Size));
		}
	}

	void TargetHighlighterController::Clear()
	{
		TargetHighlights.clear();
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

	TargetHighlightData& TargetHighlighterController::GetNewTargetHighlight(int entityID)
	{
		constexpr auto COUNT_MAX = 16;

		// Clear smallest target highlight if map is full.
		if (TargetHighlights.size() >= COUNT_MAX)
		{
			int entityIDKey = 0;
			float smallestSize = INFINITY;
			
			for (auto& [entityID, highlight] : TargetHighlights)
			{
				if (highlight.Size < smallestSize)
				{
					entityIDKey = entityID;
					smallestSize = highlight.Size;
				}
			}

			TargetHighlights.erase(entityIDKey);
		}

		// Return new target highlight.
		TargetHighlights.insert({ entityID, {} });
		auto& highlight = TargetHighlights.at(entityID);
		highlight = {};
		return highlight;
	}

	void TargetHighlighterController::AddTargetHighlight(int entityID, const Vector3& pos)
	{
		constexpr auto RADIUS_MAX = BLOCK(1);

		auto pos2D = g_Renderer.Get2DPosition(pos);
		if (!pos2D.has_value())
			return;

		// Create new target highlight.
		auto& highlight = GetNewTargetHighlight(entityID);

		highlight.IsActive = true;
		highlight.IsPrimary = false;
		highlight.Position2D = pos2D.value();
		highlight.Orientation2D = 0;
		highlight.Color = TargetHighlightData::COLOR_GRAY;
		highlight.Size = SCREEN_SPACE_RES.x / 2;
		highlight.OpacityTarget = 0.0f;
		highlight.Radius = RADIUS_MAX;
		highlight.RadiusTarget = 0.0f;
	}

	void TargetHighlighterController::ClearInactiveTargetHighlights()
	{
		for (auto it = TargetHighlights.begin(); it != TargetHighlights.end();)
		{
			if (!it->second.IsActive && (it->second.Size <= EPSILON))
			{
				it = TargetHighlights.erase(it);
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

		for (const auto& [entityID, highlight] : TargetHighlights)
			highlight.IsPrimary ? primaryCount++ : peripheralCount++;

		g_Renderer.PrintDebugMessage("Total highlights: %d", TargetHighlights.size());
		g_Renderer.PrintDebugMessage("Primary highlights: %d", primaryCount);
		g_Renderer.PrintDebugMessage("Peripheral highlights: %d", peripheralCount);
	}
}
