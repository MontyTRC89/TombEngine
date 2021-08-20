#pragma once
namespace ten::renderer {
	struct RendererPolygon {
		unsigned char Shape;
		int AnimatedSet;
		int TextureId;
		int Distance;
		int Indices[4];
	};
}
