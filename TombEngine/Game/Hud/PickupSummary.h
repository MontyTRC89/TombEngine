#pragma once
#include "Math/Math.h"
#include "Objects/game_object_ids.h"

using namespace TEN::Math;

namespace TEN::Hud
{
	struct DisplayPickup
	{
		static constexpr auto LIFE_MAX = 3.0f;

		GAME_OBJECT_ID ObjectID = ID_NO_OBJECT;
		unsigned int   Count	= 0;

		Vector2		Position2D	= Vector2::Zero;
		Vector2		Origin2D	= Vector2::Zero;
		Vector2		Target2D	= Vector2::Zero;
		EulerAngles Orientation = EulerAngles::Zero;

		float Life		   = 0.0f;
		float Scale		   = 0.0f;
		float Opacity	   = 0.0f; // BIG TODO: Object transparency in renderer.
		float HideVelocity = 0.0f;
		float StringScale  = 0.0f;
		float StringScalar = 0.0f;

		bool IsOffscreen() const;
		void Update(bool isHead);
	};

	class PickupSummaryController
	{
	private:
		// Constants
		static constexpr auto DISPLAY_PICKUP_COUNT_MAX = 64;

		// Components
		std::vector<DisplayPickup> DisplayPickups = {};

	public:
		// Utilities
		void AddDisplayPickup(GAME_OBJECT_ID objectID, const Vector3& pos);

		void Update();
		void Draw() const;
		void Clear();

	private:
		// Helpers
		std::vector<Vector2> GetStack2DPositions() const;
		DisplayPickup&		 GetNewDisplayPickup();
		void				 ClearInactiveDisplayPickups();

		void DrawDebug() const;
	};
}
