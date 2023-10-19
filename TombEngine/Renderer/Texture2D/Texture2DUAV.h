#pragma once
#include <d3d11.h>
#include <string>
#include <wrl/client.h>
#include "Renderer/TextureBase.h"

namespace TEN::Renderer
{
	using Microsoft::WRL::ComPtr;

	class Texture2DUAV : public TextureBase
	{
	public:
		int Width;
		int Height;
		ComPtr<ID3D11UnorderedAccessView> UnorderedAccessView;
		ComPtr<ID3D11Texture2D> Texture;

		Texture2DUAV() = default;
		Texture2DUAV(ID3D11Device* device, int w, int h, DXGI_FORMAT format);

		~Texture2DUAV() = default;
	};

}
