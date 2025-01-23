#include "Renderer/RendererUtils.h"

#include "Renderer/Renderer.h"
#include "Renderer/Structures/RendererShader.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/trutils.h"

namespace TEN::Renderer::Utils
{
	void ThrowIfFailed(const HRESULT& res) 
	{
		unsigned int flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

		if constexpr (DebugBuild)
		{
			flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
		}
		else
		{
			flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_IEEE_STRICTNESS;
		}

		return flags;
	}

	void ThrowIfFailed(const HRESULT& res) 
	{
		if (FAILED(res))
		{
			auto message = std::system_category().message(res);
			TENLog(message, LogLevel::Error);
			throw std::runtime_error("An error occured!");
		}
	}

	void ThrowIfFailed(const HRESULT& res, const std::string &info) 
	{
		if (FAILED(res))
		{
			auto message = info + std::system_category().message(res);
			TENLog(message, LogLevel::Error);
			throw std::runtime_error("An error occured!");
		}
	}

	void ThrowIfFailed(const HRESULT& res, const std::wstring& info) 
	{
		if (FAILED(res))
		{
			// Convert std::wstring to std::string (UTF-8).
			int size = WideCharToMultiByte(CP_UTF8, 0, info.c_str(), -1, nullptr, 0, nullptr, nullptr);
			auto utf8Info = std::string(size, 0);
			WideCharToMultiByte(CP_UTF8, 0, info.c_str(), -1, utf8Info.data(), size, nullptr, nullptr);

			// Get error message from system category and combine messages.
			auto systemMessage = std::string(std::system_category().message(res));
			auto message = utf8Info + systemMessage;

			TENLog(message, LogLevel::Error);
			throw std::runtime_error("An error occurred!");
		}
	}

	std::wstring GetAssetPath(const wchar_t* fileName)
	{
		return TEN::Utils::ToWString(g_GameFlow->GetGameDir()) + fileName;
	}
}
