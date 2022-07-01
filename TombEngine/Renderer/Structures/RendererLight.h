#pragma once
#include <SimpleMath.h>

namespace TEN::Renderer
{
	struct RendererLight
	{
		Vector3 Position;
		int Type;
		Vector3 Color;
		float LocalIntensity;
		Vector3 Direction;
		float Distance;
		float Intensity;
		float In;
		float Out;
		float Range;
		bool AffectNeighbourRooms;
		bool CastShadows;
	};
}