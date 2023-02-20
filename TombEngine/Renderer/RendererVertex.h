#pragma once
#include <SimpleMath.h>

namespace TEN::Renderer 
{
	struct RendererVertex 
	{
		Vector3 Position;
		Vector3 Normal;
		Vector2 UV;
		Vector4 Color;
		Vector3 Tangent;
		unsigned char AnimationFrameOffset;
		Vector4 Effects;
		float Bone;
		unsigned int IndexInPoly;
		unsigned int OriginalIndex;
		unsigned int Hash;
	};
}
