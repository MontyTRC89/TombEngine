#include "IndexBuffer.h"
namespace TEN::Renderer {
	using Microsoft::WRL::ComPtr;
	using TEN::Renderer::Utils::throwIfFailed;

	IndexBuffer::IndexBuffer(ID3D11Device* device, int numIndices, int* indices) {
		HRESULT res;
		D3D11_BUFFER_DESC desc = {};

		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = sizeof(int) * numIndices;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = indices;
		initData.SysMemPitch = sizeof(int) * numIndices;
		throwIfFailed(device->CreateBuffer(&desc, &initData, &Buffer));
	}

}
