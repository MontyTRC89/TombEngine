#include "framework.h"
#include "Renderer/Graphics/VertexBuffer.h"
#include "Renderer/Renderer.h"
#include "Renderer/Graphics/Vertices/Vertex.h"

namespace TEN::Renderer::Graphics
{
	using namespace TEN::Renderer::Utils;
	using namespace TEN::Renderer::Graphics::Vertices;

	VertexBuffer::VertexBuffer(ID3D11Device* device, int numVertices, Vertex* vertices)
	{
		D3D11_BUFFER_DESC desc = {};

		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = sizeof(Vertex) * numVertices;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = vertices;
		initData.SysMemPitch = sizeof(Vertex) * numVertices;
		throwIfFailed(device->CreateBuffer(&desc, &initData, &Buffer));

		_numVertices = numVertices;
	}

	VertexBuffer::VertexBuffer(ID3D11Device* device, int numVertices)
	{
		D3D11_BUFFER_DESC desc = {};

		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = sizeof(Vertex) * numVertices;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		throwIfFailed(device->CreateBuffer(&desc, nullptr, &Buffer)); 

		_numVertices = numVertices;
	}

	bool VertexBuffer::Update(ID3D11DeviceContext* context, std::vector<Vertex>& data, int startVertex, int count)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT res = context->Map(Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

		if (SUCCEEDED(res))
		{
			void* dataPtr = (mappedResource.pData);
			memcpy(dataPtr, &data[startVertex], count * sizeof(Vertex));
			context->Unmap(Buffer.Get(), 0);
			return true;
		}
		else
		{
			TENLog("Could not update constant buffer! " + std::to_string(res), LogLevel::Error);
			return false;
		}
	}
}
