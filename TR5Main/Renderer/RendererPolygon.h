#pragma once
namespace T5M::Renderer {
	struct RendererPolygon {
		unsigned char Shape;
		int AnimatedSet;
		int TextureId;
		int Distance;
		int Indices[4];
	};
}
