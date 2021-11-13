#pragma once
#include <d3d11.h>
#include "Utils.h"
#include <wrl\client.h>
namespace TEN::Renderer {
	struct RendererVertex;
	using Microsoft::WRL::ComPtr;
	class VertexBuffer {
	public:
		ComPtr<ID3D11Buffer> Buffer;
		VertexBuffer() {};
		VertexBuffer(ID3D11Device* device, int numVertices, RendererVertex* vertices);
		bool Fill(std::vector<RendererVertex> data);
	};

}



