#pragma once
namespace TEN::Renderer {
	struct RendererPolygon {
		unsigned char Shape;
		int AnimatedSet;
		int TextureId;
		int Distance;
		int Indices[4];
	};
}
