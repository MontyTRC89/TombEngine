#pragma once
#include <d3d11.h>
#include "Utils.h"
#include <wrl/client.h>
namespace TEN::Renderer {
	class IndexBuffer {
	public:
		Microsoft::WRL::ComPtr<ID3D11Buffer> Buffer;
		IndexBuffer() {};
		IndexBuffer(ID3D11Device* device, int numIndices, int* indices);
	};

}
