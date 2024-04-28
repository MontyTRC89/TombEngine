#pragma once

#include <SimpleMath.h>
#include <vector>

namespace TEN::Entities::Effects
{
	using namespace DirectX::SimpleMath;

	struct LensFlare
	{
		Vector3 Position;
		Vector3 Color;
		short RoomNumber;
		bool Global;
		int SpriteIndex;
	};

	extern std::vector<LensFlare> LensFlares;

	void LensFlareControl(short itemNumber);
	void ClearLensFlares();
	void SetupLensFlare(Vector3 position, Vector3 color, short roomNumber, bool global, int spriteIndex);
	void SetupGlobalLensFlare(Vector2 yawAndPitchInDegrees, Vector3 color, int spriteIndex);
}