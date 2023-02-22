#pragma once
#include "Math/Math.h"
#include "Objects/game_object_ids.h"

using namespace TEN::Math;

namespace TEN::Hud
{
	struct DisplayPickup
	{
		GAME_OBJECT_ID ObjectID = ID_NO_OBJECT;
		unsigned int   Count	= 0;

		Vector2		Position	= Vector2::Zero;
		Vector2		Origin		= Vector2::Zero;
		Vector2		Target		= Vector2::Zero;
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
		std::vector<Vector2> GetStackPositions() const;
		DisplayPickup&		 GetNewDisplayPickup();
		void				 ClearInactiveDisplayPickups();

		void DrawDebug() const;
	};
}
