#pragma once
#include <d3d11.h>
#include "Utils.h"
#include <wrl/client.h>

namespace TEN::Renderer
{
	struct RendererVertex;

	using Microsoft::WRL::ComPtr;

	class VertexBuffer
	{
	private:
		int m_numVertices;

	public:
		ComPtr<ID3D11Buffer> Buffer;
		VertexBuffer() {};
		VertexBuffer(ID3D11Device* device, int numVertices, RendererVertex* vertices);
		VertexBuffer(ID3D11Device* device, int numVertices);
		bool Update(ID3D11DeviceContext* device, std::vector<RendererVertex>& data, int startVertex, int count);
	};
}
