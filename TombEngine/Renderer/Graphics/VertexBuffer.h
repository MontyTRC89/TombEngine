#pragma once
#include <d3d11.h>
#include "Renderer/RendererUtils.h"
#include "Renderer/Graphics/Vertices/Vertex.h"
#include <wrl/client.h>

using namespace TEN::Renderer::Graphics::Vertices;

namespace TEN::Renderer::Graphics
{
	using Microsoft::WRL::ComPtr;

	class VertexBuffer
	{
	private:
		int m_numVertices;

	public:
		ComPtr<ID3D11Buffer> Buffer;
		VertexBuffer() {};
		VertexBuffer(ID3D11Device* device, int numVertices, Vertex* vertices);
		VertexBuffer(ID3D11Device* device, int numVertices);
		bool Update(ID3D11DeviceContext* device, std::vector<Vertex>& data, int startVertex, int count);
	};
}
