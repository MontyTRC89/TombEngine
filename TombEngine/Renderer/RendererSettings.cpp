#include "framework.h"
#include <filesystem>
#include <codecvt>

#include "Renderer/Renderer.h"
#include "Specific/trutils.h"
#include "Specific/winmain.h"

namespace TEN::Renderer 
{
	void Renderer::ChangeScreenResolution(int width, int height, bool windowed) 
	{
		ID3D11RenderTargetView* nullViews[] = { nullptr };
		context->OMSetRenderTargets(0, nullViews, NULL);
		context->Flush();
		context->ClearState();

		IDXGIOutput* output;
		Utils::throwIfFailed(swapChain->GetContainingOutput(&output));

		DXGI_SWAP_CHAIN_DESC scd;
		Utils::throwIfFailed(swapChain->GetDesc(&scd));

		unsigned int numModes = 1024;
		DXGI_MODE_DESC modes[1024];
		Utils::throwIfFailed(output->GetDisplayModeList(scd.BufferDesc.Format, 0, &numModes, modes));

		DXGI_MODE_DESC* mode = &modes[0];
		for (unsigned int i = 0; i < numModes; i++)
		{
			mode = &modes[i];
			if (mode->Width == width && mode->Height == height)
				break;
		}

		Utils::throwIfFailed( swapChain->ResizeTarget(mode));

		screenWidth = width;
		screenHeight = height;
		isWindowed = windowed;

		InitializeScreen(width, height, WindowsHandle, true);
	}

	std::string Renderer::GetDefaultAdapterName()
	{
		IDXGIFactory* dxgiFactory = NULL;
		Utils::throwIfFailed(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

		IDXGIAdapter* dxgiAdapter = NULL;

		dxgiFactory->EnumAdapters(0, &dxgiAdapter);

		DXGI_ADAPTER_DESC adapterDesc = {};

		dxgiAdapter->GetDesc(&adapterDesc);
		dxgiFactory->Release();
		
		return TEN::Utils::ToString(adapterDesc.Description);
	}

	void Renderer::SetTextureOrDefault(Texture2D& texture, std::wstring path)
	{
		texture = Texture2D();

		if (std::filesystem::is_regular_file(path))
			texture = Texture2D(device.Get(), path);
		else
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
			TENLog("Texture file not found: " + converter.to_bytes(path), LogLevel::Warning);
		}
	}
}
