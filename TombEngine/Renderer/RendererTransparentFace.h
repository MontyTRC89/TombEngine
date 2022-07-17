#pragma once
#include <SimpleMath.h>
#include "Renderer/Renderer11Enums.h"

struct MESH_INFO;

namespace TEN::Renderer
{
	struct RendererBucket;
	struct RendererPolygon;
	struct RendererSpriteToDraw;
	struct RendererRoom;
	struct RendererItem;
	struct RendererStatic;
	struct RendererObject;

	struct RendererTransparentFaceInfo
	{
		RendererPolygon* polygon;
		RendererSpriteToDraw* sprite;
		RendererRoom* room;
		RendererBucket* bucket;
		RendererItem* item;
		RendererStatic* staticMesh;
		Vector4 color;
		Matrix world;
		Vector3 position;
		int texture;
		int bone;
		bool animated;
		bool doubleSided;
		BLEND_MODES blendMode;
	};

	struct RendererTransparentFace
	{
		RendererTransparentFaceType type;
		int distance;
		RendererTransparentFaceInfo info;

		RendererTransparentFace()
		{

		}

		RendererTransparentFace(const RendererTransparentFace& source)
		{
			type = source.type;
			distance = source.distance;
			info.animated = source.info.animated;
			info.blendMode = source.info.blendMode;
			info.color = source.info.color;
			info.doubleSided = source.info.doubleSided;
			info.item = source.info.item;
			info.polygon = source.info.polygon;
			info.position = source.info.position;
			info.room = source.info.room;
			info.sprite = source.info.sprite;
			info.staticMesh = source.info.staticMesh;
			info.texture = source.info.texture;
			info.world = source.info.world;
			info.bucket = source.info.bucket;
		}

		RendererTransparentFace(RendererTransparentFace& source)
		{
			type = source.type;
			distance = source.distance;
			info.animated = source.info.animated;
			info.blendMode = source.info.blendMode;
			info.color = source.info.color;
			info.doubleSided = source.info.doubleSided;
			info.item = source.info.item;
			info.polygon = source.info.polygon;
			info.position = source.info.position;
			info.room = source.info.room;
			info.sprite = source.info.sprite;
			info.staticMesh = source.info.staticMesh;
			info.texture = source.info.texture;
			info.world = source.info.world;
			info.bucket = source.info.bucket;
		}
	};
}
