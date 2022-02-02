#pragma once
#include <d3d11.h>
#include <string>
#include <wrl/client.h>
#include "Renderer/TextureBase.h"

namespace TEN::Renderer 
{
	using Microsoft::WRL::ComPtr;

	class Texture2D : public TextureBase
	{
	public:
		ComPtr<ID3D11Texture2D> Texture;
		Texture2D() = default;
		Texture2D(ID3D11Device* device, int w, int h, byte* data);
		Texture2D(ID3D11Device* device, const std::wstring& fileName);

		~Texture2D() = default;

		Texture2D(ID3D11Device* device, byte* data, int length);
	};

}
