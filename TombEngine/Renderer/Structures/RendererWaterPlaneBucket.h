#pragma once
#include <vector>
#include <SimpleMath.h>
#include "Renderer/Structures/RendererRoom.h"
#include "Renderer/Structures/RendererBucket.h"

namespace TEN::Renderer::Structures
{
	using namespace DirectX::SimpleMath;

	struct RendererWaterPlaneBucket
	{
		RendererRoom* Room;
		RendererBucket* Bucket;
	};

	struct RendererWaterPlane
	{
		int WaterLevel;
		Matrix ReflectionViewMatrix;
		Vector3 CameraPositionWS;
		Vector3 CameraDirectionWS;
		std::vector<RendererWaterPlaneBucket> Buckets;
	};
}
