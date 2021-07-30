#pragma once
#include <SimpleMath.h>
namespace T5M::Renderer {
	struct RendererVertex {
		DirectX::SimpleMath::Vector3 Position;
		DirectX::SimpleMath::Vector3 Normal;
		DirectX::SimpleMath::Vector2 UV;
		DirectX::SimpleMath::Vector4 Color;
		DirectX::SimpleMath::Vector3 Tangent;
		DirectX::SimpleMath::Vector3 BiTangent;
		float Bone;
		int IndexInPoly;
		int OriginalIndex;
		int Effects;
		int hash;
	};
}
