#pragma once
#include <SimpleMath.h>

namespace TEN::Renderer
{
	struct RendererLight
	{
		Vector3 Position;
		unsigned int Type;
		Vector3 Color;
		float Intensity;
		Vector3 Direction;
		float In;
		float Out;
		float InRange;
		float OutRange;

		int RoomNumber;
		float LocalIntensity;
		float Distance;
		bool AffectNeighbourRooms;
		bool CastShadows;
	};
}