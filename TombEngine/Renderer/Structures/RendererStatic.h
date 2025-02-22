#pragma once

#include "Math/Objects/GameBoundingBox.h"
#include "Math/Objects/Pose.h"
#include "Renderer/Structures/RendererLight.h"

namespace TEN::Renderer::Structures
{
	struct RendererStatic
	{
		int ObjectNumber;
		int RoomNumber;
		int IndexInRoom;

		Pose	Pose;
		Matrix	World;
		Vector4 Color;
		Vector4 AmbientLight;

		std::vector<RendererLight*>	   LightsToDraw;
		std::vector<RendererLightNode> CachedRoomLights;
		bool						   CacheLights;

		BoundingSphere OriginalSphere;
		BoundingSphere Sphere;

		void Update()
		{
			auto translationMatrix = Matrix::CreateTranslation(Pose.Position.ToVector3());
			auto rotMatrix = Pose.Orientation.ToRotationMatrix();
			auto scaleMatrix = Matrix::CreateScale(Pose.Scale);
			auto worldMatrix = rotMatrix * scaleMatrix * translationMatrix;

			auto sphereCenter = Vector3::Transform(OriginalSphere.Center, World);
			float sphereScale = std::max({ Pose.Scale.x, Pose.Scale.y, Pose.Scale.z });
			float sphereRadius = OriginalSphere.Radius * sphereScale;

			World = worldMatrix;
			Sphere = BoundingSphere(sphereCenter, sphereRadius);
			CacheLights = true;
		}
	};
}
