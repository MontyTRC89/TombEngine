#pragma once
#include <d3d11.h>
#include <string>
#include <wrl/client.h>

namespace TEN::Renderer
{
	using Microsoft::WRL::ComPtr;

	class TextureBase
	{
	public:
		ComPtr<ID3D11ShaderResourceView> ShaderResourceView;
	};
}
