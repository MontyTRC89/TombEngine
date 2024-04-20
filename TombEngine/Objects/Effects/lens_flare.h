#pragma once

#include <SimpleMath.h>
#include <vector>

namespace TEN::Entities::Effects
{
	using namespace DirectX::SimpleMath;

	struct LensFlare
	{
		Vector3 Position;
		short RoomNumber;
	};

	extern std::vector<LensFlare> LensFlares;

	void LensFlareControl(short itemNumber);
	void ClearLensFlares();
}