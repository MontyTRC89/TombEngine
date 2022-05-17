#pragma once
#include <SimpleMath.h>

namespace TEN::Renderer 
{
	struct RendererVertex 
	{
		DirectX::SimpleMath::Vector3 Position;
		DirectX::SimpleMath::Vector3 Normal;
		DirectX::SimpleMath::Vector2 UV;
		DirectX::SimpleMath::Vector4 Color;
		DirectX::SimpleMath::Vector3 Tangent;
		DirectX::SimpleMath::Vector3 BiTangent;
		DirectX::SimpleMath::Vector3 Effects;
		float Bone;
		int IndexInPoly;
		int OriginalIndex;
		int hash;
	};
}
