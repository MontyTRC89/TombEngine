#pragma once
#include <d3d11.h>
#include <vector>
#include <wrl/client.h>
#include "Utils.h"
#include "Game/debug/debug.h"
#include "Specific/fast_vector.h"

namespace TEN::Renderer {
	class IndexBuffer {
	private:
		int m_numIndices;

	public:
		Microsoft::WRL::ComPtr<ID3D11Buffer> Buffer;
		IndexBuffer() {};
		IndexBuffer(ID3D11Device* device, int numIndices, int* indices);
		IndexBuffer(ID3D11Device* device, int numIndices);
		bool Update(ID3D11DeviceContext* context, std::vector<int>& data, int startIndex, int count);
		bool Update(ID3D11DeviceContext* context, fast_vector<int>& data, int startIndex, int count);
	};

}
