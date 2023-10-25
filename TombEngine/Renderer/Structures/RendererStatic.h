#pragma once
#include <vector>
#include <SimpleMath.h>
#include "Math/Objects/GameBoundingBox.h"
#include "Math/Objects/Pose.h"
#include "Renderer/Structures/RendererLight.h"

namespace TEN::Renderer::Structures
{
	using namespace DirectX;
	using namespace DirectX::SimpleMath;

	struct RendererStatic
	{
		int ObjectNumber;
		int RoomNumber;
		int IndexInRoom;
		Pose Pose;
		Matrix World;
		Vector4 Color;
		Vector4 AmbientLight;
		std::vector<RendererLight*> LightsToDraw;
		std::vector<RendererLight*> CachedRoomLights;
		bool CacheLights;
		GameBoundingBox OriginalVisibilityBox;
		BoundingOrientedBox VisibilityBox;
		float Scale;

		void Update()
		{
			World = (Pose.Orientation.ToRotationMatrix() *
				Matrix::CreateScale(Scale) *
				Matrix::CreateTranslation(Pose.Position.x, Pose.Position.y, Pose.Position.z));
			CacheLights = true;
			VisibilityBox = OriginalVisibilityBox.ToBoundingOrientedBox(Pose);
		}
	};
}