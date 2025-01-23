#include "Renderer/Renderer.h"

#include "Specific/trutils.h"
#include "Specific/winmain.h"

namespace TEN::Renderer 
{
	void Renderer::ChangeScreenResolution(int width, int height, bool windowed) 
	{
		ID3D11RenderTargetView* nullViews[] = { nullptr };
		_context->OMSetRenderTargets(0, nullViews, NULL);
		_context->Flush();
		_context->ClearState();

		IDXGIOutput* output;
		Utils::ThrowIfFailed(_swapChain->GetContainingOutput(&output));

		DXGI_SWAP_CHAIN_DESC scd;
		Utils::ThrowIfFailed(_swapChain->GetDesc(&scd));

		unsigned int numModes = 1024;
		DXGI_MODE_DESC modes[1024];
		Utils::ThrowIfFailed(output->GetDisplayModeList(scd.BufferDesc.Format, 0, &numModes, modes));

		DXGI_MODE_DESC* mode = &modes[0];
		for (unsigned int i = 0; i < numModes; i++)
		{
			mode = &modes[i];
			if (mode->Width == width && mode->Height == height)
				break;
		}

		Utils::ThrowIfFailed( _swapChain->ResizeTarget(mode));

		_screenWidth = width;
		_screenHeight = height;
		_isWindowed = windowed;

		InitializeScreen(width, height, WindowsHandle, true);
	}

	std::string Renderer::GetDefaultAdapterName()
	{
		IDXGIFactory* dxgiFactory = NULL;
		Utils::ThrowIfFailed(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

		IDXGIAdapter* dxgiAdapter = NULL;

		dxgiFactory->EnumAdapters(0, &dxgiAdapter);

		DXGI_ADAPTER_DESC adapterDesc = {};

		dxgiAdapter->GetDesc(&adapterDesc);
		dxgiFactory->Release();
		
		return TEN::Utils::ToString(adapterDesc.Description);
	}

	void Renderer::SetTextureOrDefault(Texture2D& texture, const std::wstring& path)
	{
		texture = Texture2D();

		if (std::filesystem::is_regular_file(path))
		{
			texture = Texture2D(_device.Get(), path);
		}
		else if (!path.empty()) // Loading default texture without path may be intentional.
		{
			// Convert std::wstring to std::string (UTF-8).
			int size = WideCharToMultiByte(CP_UTF8, 0, path.c_str(), -1, nullptr, 0, nullptr, nullptr);
			auto utf8Path = std::string(size, 0);
			WideCharToMultiByte(CP_UTF8, 0, path.c_str(), -1, utf8Path.data(), size, nullptr, nullptr);

			TENLog("Texture file not found: " + utf8Path, LogLevel::Warning);
		}
	}
}
