#include "framework.h"
#include "VertexBuffer.h"
#include "Renderer11.h"
namespace TEN::Renderer {
	using namespace TEN::Renderer::Utils;

	VertexBuffer::VertexBuffer(ID3D11Device* device, int numVertices, RendererVertex* vertices) {
		HRESULT res;
		D3D11_BUFFER_DESC desc = {};

		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = sizeof(RendererVertex) * numVertices;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = vertices;
		initData.SysMemPitch = sizeof(RendererVertex) * numVertices;
		throwIfFailed(device->CreateBuffer(&desc, &initData, &Buffer));
	}

	VertexBuffer::VertexBuffer(ID3D11Device* device, int numVertices) {
		HRESULT res;
		D3D11_BUFFER_DESC desc = {};

		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = sizeof(RendererVertex) * numVertices;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		throwIfFailed(device->CreateBuffer(&desc, nullptr, &Buffer)); 
	}

	bool VertexBuffer::Update(ID3D11DeviceContext* context, std::vector<RendererVertex>* data)
	{
		D3D11_MAPPED_SUBRESOURCE resource;
		context->Map(Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		memcpy(resource.pData, data->data(), data->size() * sizeof(RendererVertex));
		context->Unmap(Buffer.Get(), 0);

		return true;
	}
}
