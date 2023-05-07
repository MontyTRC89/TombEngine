#pragma once

namespace TEN::Renderer
{
	using Microsoft::WRL::ComPtr;

	class TextureBase
	{
	public:
		ComPtr<ID3D11ShaderResourceView> ShaderResourceView;
	};
}
