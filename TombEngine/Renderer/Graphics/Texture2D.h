#pragma once
#include <d3d11.h>
#include <string>
#include <wrl/client.h>
#include "Renderer/Graphics/TextureBase.h"

namespace TEN::Renderer::Graphics
{
	using Microsoft::WRL::ComPtr;

	class Texture2D : public TextureBase
	{
	public:
		int Width;
		int Height;

		ComPtr<ID3D11Texture2D> Texture;

		Texture2D() = default;
		Texture2D(ID3D11Device* device, int width, int height, byte* data);
		Texture2D(ID3D11Device* device, int width, int height, DXGI_FORMAT format, int pitch, const void* data);
		Texture2D(ID3D11Device* device, const std::wstring& fileName);
		Texture2D(ID3D11Device* device, byte* data, int length);

		~Texture2D() = default;
	};
}
