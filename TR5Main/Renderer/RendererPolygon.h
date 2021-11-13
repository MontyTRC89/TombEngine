#pragma once
namespace TEN::Renderer {
	struct RendererPolygon {
		Vector3 centre;
		unsigned char shape;
		std::vector<RendererVertex> vertices;
	};
}
