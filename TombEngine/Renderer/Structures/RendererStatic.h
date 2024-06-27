#pragma once

#include "Renderer/Structures/RendererLight.h"

namespace TEN::Renderer::Structures
{
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
		std::vector<RendererLightNode> CachedRoomLights;
		bool CacheLights;
		GameBoundingBox OriginalVisibilityBox;
		BoundingOrientedBox VisibilityBox;
		float Scale;
		float VisibilitySphereRadius;

		void Update()
		{
			World = (Pose.Orientation.ToRotationMatrix() *
				Matrix::CreateScale(Scale) *
				Matrix::CreateTranslation(Pose.Position.x, Pose.Position.y, Pose.Position.z));
			CacheLights = true;
			VisibilityBox = OriginalVisibilityBox.ToBoundingOrientedBox(Pose);
			VisibilitySphereRadius = Vector3(VisibilityBox.Extents).Length() * Scale;
		}
	};
}
