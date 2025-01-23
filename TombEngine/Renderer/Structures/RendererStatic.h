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
		BoundingSphere OriginalSphere;
		BoundingSphere Sphere;
		float Scale;

		void Update()
		{
			World = (Pose.Orientation.ToRotationMatrix() *
				Matrix::CreateScale(Scale) *
				Matrix::CreateTranslation(Pose.Position.x, Pose.Position.y, Pose.Position.z));
			Sphere = BoundingSphere(Vector3::Transform(OriginalSphere.Center, World), OriginalSphere.Radius * Scale);
			CacheLights = true;
		}
	};
}
