#pragma once
#include <wrl/client.h>
#include <d3d11.h>

namespace TEN::Renderer
{
	using Microsoft::WRL::ComPtr;

	extern ComPtr<ID3D11Buffer> quadVertexBuffer;

	void initQuad(ID3D11Device* device);
}
