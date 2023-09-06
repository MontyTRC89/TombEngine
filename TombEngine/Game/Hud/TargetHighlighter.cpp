#include "framework.h"
#include "Game/Hud/TargetHighlighter.h"

#include "Game/camera.h"
#include "Game/items.h"
#include "Game/effects/ScreenSprite.h"
#include "Game/lara/lara_fire.h"
#include "Game/lara/lara_helpers.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"
#include "Specific/configuration.h"
#include "Specific/trutils.h"

using namespace TEN::Effects::ScreenSprite;
using namespace TEN::Math;
using namespace TEN::Utils;
using TEN::Renderer::g_Renderer;

namespace TEN::Hud
{
	// TODO: Not working?
	bool CrosshairData::IsOffscreen() const
	{
		float screenEdgeThreshold = GetRadius();

		return (Position.x <= -screenEdgeThreshold ||
				Position.y <= -screenEdgeThreshold ||
				Position.x >= (SCREEN_SPACE_RES.x + screenEdgeThreshold) ||
				Position.y >= (SCREEN_SPACE_RES.y + screenEdgeThreshold));
	}

	float CrosshairData::GetSize(float cameraDist) const
	{
		constexpr auto RANGE			  = BLOCK(10);
		constexpr auto CROSSHAIR_SIZE_MAX = SCREEN_SPACE_RES.y * 0.15f;
		constexpr auto CROSSHAIR_SIZE_MIN = CROSSHAIR_SIZE_MAX / 3;

		auto alpha = cameraDist / RANGE;
		return Lerp(CROSSHAIR_SIZE_MAX, CROSSHAIR_SIZE_MIN, alpha);
	}

	float CrosshairData::GetRadius() const
	{
		return ((Size / 2) * (RadiusScale * PulseScale));
	}

	Vector2 CrosshairData::GetPositionOffset(short orientOffset) const
	{
		constexpr auto ANGLE_OFFSET = ANGLE(-360.0f / (SEGMENT_COUNT * 2));

		float offsetDist = GetRadius();
		auto relPosOffset = Vector2(offsetDist, 0.0f);
		auto rotMatrix = Matrix::CreateRotationZ(TO_RAD(Orientation + orientOffset + ANGLE_OFFSET));

		auto posOffset2D = Vector2::Transform(relPosOffset, rotMatrix);
		return GetAspectCorrect2DPosition(posOffset2D);
	}

	void CrosshairData::SetPrimary()
	{
		IsPrimary = true;
		ColorTarget = COLOR_RED;
	}

	void CrosshairData::SetPeripheral()
	{
		IsPrimary = false;
		ColorTarget = COLOR_GRAY;
	}

	void CrosshairData::Update(const Vector3& targetPos, bool doPulse, bool isActive)
	{
		constexpr auto INVALID_POS			   = Vector2(FLT_MAX);
		constexpr auto ROT					   = ANGLE(2.0f);
		constexpr auto ALIGN_ANGLE_STEP		   = ANGLE(360.0f / SEGMENT_COUNT);
		constexpr auto SIZE_SCALE_PRIMARY	   = 1.0f;
		constexpr auto SIZE_SCALE_PERIPHERAL   = 0.75f;
		constexpr auto RADIUS_SCALE_PRIMARY	   = 0.5f * SQRT_2;
		constexpr auto RADIUS_SCALE_PERIPHERAL = 0.25f * SQRT_2;
		constexpr auto PULSE_SCALE_MAX		   = 1.3f;
		constexpr auto MORPH_LERP_ALPHA		   = 0.3f;
		constexpr auto ORIENT_LERP_ALPHA	   = 0.1f;
		constexpr auto RADIUS_LERP_ALPHA	   = 0.2f;

		// Update active status.
		IsActive = isActive;

		// Update position.
		auto pos = g_Renderer.Get2DPosition(targetPos);
		Position = pos.has_value() ? pos.value() : INVALID_POS;

		// Update orientation.
		if (IsPrimary)
		{
			Orientation += ROT;
		}
		else
		{
			short closestAlignAngle = (Orientation / ALIGN_ANGLE_STEP) * ALIGN_ANGLE_STEP;
			Orientation = (short)round(Lerp(Orientation, closestAlignAngle, ORIENT_LERP_ALPHA));
		}

		// Update size.
		if (IsActive)
		{
			float cameraDist = Vector3::Distance(Camera.pos.ToVector3(), targetPos);
			float sizeTarget = GetSize(cameraDist) * (IsPrimary ? SIZE_SCALE_PRIMARY : SIZE_SCALE_PERIPHERAL);

			Size = Lerp(Size, sizeTarget, MORPH_LERP_ALPHA);
		}
		else
		{
			Size = Lerp(Size, 0.0f, MORPH_LERP_ALPHA / 2);
		}

		// Update color.
		if (!IsActive)
		{
			ColorTarget = CrosshairData::COLOR_GRAY;
			ColorTarget.w = 0.0f;
		}
		Color = Vector4::Lerp(Color, ColorTarget, MORPH_LERP_ALPHA);

		// Update radius scale.
		float radiusScaleTarget = IsPrimary ? RADIUS_SCALE_PRIMARY : RADIUS_SCALE_PERIPHERAL;
		if (!IsActive)
			radiusScaleTarget = RADIUS_SCALE_PERIPHERAL;

		RadiusScale = Lerp(RadiusScale, radiusScaleTarget, RADIUS_LERP_ALPHA);

		// Update pulse scale.
		PulseScale = doPulse ? PULSE_SCALE_MAX : Lerp(PulseScale, 1.0f, MORPH_LERP_ALPHA);

		// Update segments.
		for (auto& segment : Segments)
			segment.PosOffset = GetPositionOffset(segment.OrientOffset);
	}

	void TargetHighlighterController::Update(const ItemInfo& playerItem)
	{
		// Check if target highlighter is enabled.
		if (!g_Configuration.EnableTargetHighlighter)
		{
			if (!Crosshairs.empty())
				Crosshairs.clear();

			return;
		}

		const auto& player = GetLaraInfo(playerItem);

		// Loop over player targets.
		auto itemNumbers = std::vector<int>{};
		for (const auto* itemPtr : player.TargetList)
		{
			if (itemPtr == nullptr)
				continue;

			// Collect item number.
			itemNumbers.push_back(itemPtr->Index);

			// Find crosshair at item number key.
			auto it = Crosshairs.find(itemPtr->Index);
			if (it == Crosshairs.end())
				continue;

			// Set crosshair as primary or peripheral.
			auto& crosshair = it->second;
			if (player.TargetEntity != nullptr &&
				itemPtr->Index == player.TargetEntity->Index)
			{
				crosshair.SetPrimary();
			}
			else
			{
				crosshair.SetPeripheral();
			}
		}

		// Update crosshairs.
		Update(itemNumbers);
	}

	void TargetHighlighterController::Draw() const
	{
		constexpr auto CROSSHAIR_SPRITE_STATIC_ELEMENT_INDEX  = 0;
		constexpr auto CROSSHAIR_SPRITE_SEGMENT_ELEMENT_INDEX = 1;

		DrawDebug();

		if (Crosshairs.empty())
			return;

		for (const auto& [itemNumber, crosshair] : Crosshairs)
		{
			if (crosshair.IsOffscreen())
				continue;

			AddScreenSprite(
				ID_CROSSHAIR, CROSSHAIR_SPRITE_STATIC_ELEMENT_INDEX,
				crosshair.Position, crosshair.Orientation,
				Vector2(crosshair.Size), crosshair.Color, 0, BLEND_MODES::BLENDMODE_ALPHABLEND);

			if (crosshair.RadiusScale > EPSILON)
			{
				for (const auto& segment : crosshair.Segments)
				{
					AddScreenSprite(
						ID_CROSSHAIR, CROSSHAIR_SPRITE_SEGMENT_ELEMENT_INDEX,
						crosshair.Position + segment.PosOffset, crosshair.Orientation + segment.OrientOffset,
						Vector2(crosshair.Size / 2), crosshair.Color, 0, BLEND_MODES::BLENDMODE_ALPHABLEND);
				}
			}
		}
	}

	void TargetHighlighterController::Clear()
	{
		*this = {};
	}

	void TargetHighlighterController::Update(const std::vector<int>& itemNumbers)
	{
		constexpr auto TARGET_BONE_ID = 0;

		// No crosshairs to update; return early.
		if (Crosshairs.empty() && itemNumbers.empty())
			return;

		// Update active crosshairs.
		for (int itemNumber : itemNumbers)
		{
			const auto& item = g_Level.Items[itemNumber];
			auto targetPos = GetJointPosition(item, TARGET_BONE_ID).ToVector3();

			// Update existing active crosshair.
			auto it = Crosshairs.find(itemNumber);
			if (it != Crosshairs.end())
			{
				auto& crosshair = it->second;
				if (crosshair.IsActive)
				{
					crosshair.Update(targetPos, item.HitStatus, true);
					continue;
				}
			}

			// Add new active crosshair.
			AddCrosshair(itemNumber, targetPos);
		}

		// Update inactive crosshairs.
		for (auto& [itemNumber, crosshair] : Crosshairs)
		{
			// Find crosshairs at absent item number keys.
			if (Contains(itemNumbers, itemNumber))
				continue;

			const auto& item = g_Level.Items[itemNumber];
			auto targetPos = GetJointPosition(item, 0).ToVector3();

			// Update inactive crosshair.
			crosshair.Update(targetPos, item.HitStatus, false);
		}

		ClearInactiveCrosshairs();
	}

	CrosshairData& TargetHighlighterController::GetNewCrosshair(int itemNumber)
	{
		constexpr auto COUNT_MAX = 16;

		// Map is full; clear smallest crosshair.
		if (Crosshairs.size() >= COUNT_MAX)
		{
			int key = 0;
			float smallestSize = INFINITY;
			
			for (auto& [itemNumber, crosshair] : Crosshairs)
			{
				if (crosshair.Size < smallestSize)
				{
					key = itemNumber;
					smallestSize = crosshair.Size;
				}
			}

			Crosshairs.erase(key);
		}

		// Return new crosshair.
		Crosshairs.insert({ itemNumber, {} });
		auto& crosshair = Crosshairs.at(itemNumber);
		crosshair = {};
		return crosshair;
	}

	// TODO: If crosshair happens to be in view upon spawn, first frame is sometimes garbage.
	void TargetHighlighterController::AddCrosshair(int itemNumber, const Vector3& targetPos)
	{
		constexpr auto SIZE_START		  = SCREEN_SPACE_RES.x / 2;
		constexpr auto RADIUS_SCALE_START = 1.5f * SQRT_2;
		constexpr auto ANGLE_STEP		  = ANGLE(360.0f / CrosshairData::SEGMENT_COUNT);

		auto pos = g_Renderer.Get2DPosition(targetPos);
		if (!pos.has_value())
			return;

		// Create new crosshair.
		auto& crosshair = GetNewCrosshair(itemNumber);

		crosshair.IsActive = true;
		crosshair.IsPrimary = false;
		crosshair.Position = pos.value();
		crosshair.Orientation = 0;
		crosshair.Size = SIZE_START;
		crosshair.Color = CrosshairData::COLOR_GRAY;
		crosshair.Color.w = 0.0f;
		crosshair.ColorTarget = CrosshairData::COLOR_GRAY;
		crosshair.RadiusScale = RADIUS_SCALE_START;
		crosshair.PulseScale = 1.0f;

		// Initialize segments.
		short angleOffset = 0;
		for (auto& segment : crosshair.Segments)
		{
			segment.PosOffset = crosshair.GetPositionOffset(segment.OrientOffset);
			segment.OrientOffset = angleOffset;

			angleOffset += ANGLE_STEP;
		}
	}

	void TargetHighlighterController::ClearInactiveCrosshairs()
	{
		for (auto it = Crosshairs.begin(); it != Crosshairs.end();)
		{
			const auto& crosshair = it->second;
			(!crosshair.IsActive && crosshair.Size <= EPSILON) ?
				(it = Crosshairs.erase(it)) : ++it;
		}
	}

	void TargetHighlighterController::DrawDebug() const
	{
		unsigned int primaryCount = 0;
		unsigned int peripheralCount = 0;
		unsigned int visibleCount = 0;

		for (const auto& [itemNumber, crosshair] : Crosshairs)
		{
			crosshair.IsPrimary ? primaryCount++ : peripheralCount++;
			
			if (!crosshair.IsOffscreen())
				visibleCount++;
		}

		g_Renderer.PrintDebugMessage("TARGET HIGHLIGHTER DEBUG");
		g_Renderer.PrintDebugMessage(g_Configuration.EnableTargetHighlighter ? "Enabled" : "Disabled");
		g_Renderer.PrintDebugMessage("Primary crosshairs: %d", primaryCount);
		g_Renderer.PrintDebugMessage("Peripheral crosshairs: %d", peripheralCount);
		g_Renderer.PrintDebugMessage("Visible crosshairs: %d", peripheralCount);
	}
}
