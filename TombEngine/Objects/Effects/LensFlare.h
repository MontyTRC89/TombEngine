#pragma once

namespace TEN::Entities::Effects
{
	struct LensFlare
	{
		int SpriteID = 0;

		Vector3 Position   = Vector3::Zero;
		int		RoomNumber = 0;	
		Color	Color	   = {};

		bool IsGlobal = false;
	};

	extern std::vector<LensFlare> LensFlares;

	void ControlLensFlare(int itemNumber);
	void ClearLensFlares();
	void UpdateGlobalLensFlare();
}
