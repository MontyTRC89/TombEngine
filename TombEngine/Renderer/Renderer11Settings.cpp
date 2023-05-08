#include "framework.h"
#include <filesystem>
#include <codecvt>

#include "Renderer/Renderer11.h"
#include "Specific/trutils.h"
#include "Specific/winmain.h"

namespace TEN::Renderer 
{
	void Renderer11::ChangeScreenResolution(int width, int height, bool windowed) 
	{
		ID3D11RenderTargetView* nullViews[] = { nullptr };
		m_context->OMSetRenderTargets(0, nullViews, NULL);

		m_backBufferTexture->Release();
		m_backBufferRTV->Release();
		m_depthStencilView->Release();
		m_depthStencilTexture->Release();
		m_context->Flush();
		m_context->ClearState();

		IDXGIOutput* output;
		Utils::throwIfFailed(m_swapChain->GetContainingOutput(&output));

		DXGI_SWAP_CHAIN_DESC scd;
		Utils::throwIfFailed(m_swapChain->GetDesc(&scd));

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

		Utils::throwIfFailed( m_swapChain->ResizeTarget(mode));

		m_screenWidth = width;
		m_screenHeight = height;
		m_windowed = windowed;

		InitializeScreen(width, height, WindowsHandle, true);
	}

	std::string Renderer11::GetDefaultAdapterName()
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

	void Renderer11::SetTextureOrDefault(Texture2D& texture, std::wstring path)
	{
		texture = Texture2D();

		if (std::filesystem::exists(path))
			texture = Texture2D(m_device.Get(), path);
		else
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
			TENLog("Texture file not found: " + converter.to_bytes(path), LogLevel::Warning);
		}
	}
}
