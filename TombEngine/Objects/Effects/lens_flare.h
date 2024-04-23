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
		bool Sun;
	};

	extern std::vector<LensFlare> LensFlares;

	void LensFlareControl(short itemNumber);
	void ClearLensFlares();
	void SetupLensFlare(Vector3 position, short roomNumber, bool global);
	void SetupGlobalLensFlare(float yaw, float pitch);
}