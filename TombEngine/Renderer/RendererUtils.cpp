#include "Renderer/RendererUtils.h"

#include "Renderer/Renderer.h"
#include "Specific/trutils.h"

namespace TEN::Renderer::Utils
{
	using Microsoft::WRL::ComPtr;

	constexpr unsigned int Utils::GetShaderFlags()
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

	ComPtr<ID3D11VertexShader> CompileVertexShader(ID3D11Device* device, const std::wstring& fileName, const std::string& function, const std::string& model, const D3D_SHADER_MACRO * defines, ComPtr<ID3D10Blob>& bytecode) 
	{
		ComPtr<ID3D10Blob> errors;
		HRESULT res = (D3DCompileFromFile(fileName.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, function.c_str(), model.c_str(), GetShaderFlags(), 0, bytecode.GetAddressOf(),errors.GetAddressOf()));
		if (FAILED(res))
		{
			ID3D10Blob* errorObj = errors.Get();
			if (errorObj != nullptr)
			{
				auto error = std::string((char*)errorObj->GetBufferPointer());
				TENLog(error, LogLevel::Error);
				throw std::runtime_error(error);
			} 
			else
			{
				TENLog("Error while compiling VS shader: " + TEN::Utils::ToString(fileName.c_str()), LogLevel::Error);
				ThrowIfFailed(res);
			}
		}

		ComPtr<ID3D11VertexShader> shader;
		ThrowIfFailed(device->CreateVertexShader(bytecode->GetBufferPointer(), bytecode->GetBufferSize(), nullptr, shader.GetAddressOf()));
		
		if constexpr (DebugBuild)
		{
			char buffer[100];
			unsigned int size = (unsigned int)std::wcstombs(buffer, fileName.c_str(), 100);
			shader->SetPrivateData(WKPDID_D3DDebugObjectName, size, buffer);
		}

		return shader;
	}

	ComPtr<ID3D11PixelShader> CompilePixelShader(ID3D11Device* device, const std::wstring& fileName, const std::string& function, const std::string& model, const D3D_SHADER_MACRO* defines, ComPtr<ID3D10Blob>& bytecode)
	{
		ComPtr<ID3D10Blob> errors;
		HRESULT res = (D3DCompileFromFile(fileName.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, function.c_str(), model.c_str(), GetShaderFlags(), 0, bytecode.GetAddressOf(), errors.GetAddressOf()));
		if (FAILED(res))
		{
			ID3D10Blob* errorObj = errors.Get();
			if (errorObj != nullptr)
			{
				auto error = std::string((char*)errorObj->GetBufferPointer());
				TENLog(error, LogLevel::Error);
				throw std::runtime_error(error);
			}
			else
			{
				TENLog("Error while compiling PS shader: " + TEN::Utils::ToString(fileName.c_str()), LogLevel::Error);
				ThrowIfFailed(res);
			}
		}

		ComPtr<ID3D11PixelShader> shader;
		ThrowIfFailed(device->CreatePixelShader(bytecode->GetBufferPointer(), bytecode->GetBufferSize(), nullptr, shader.GetAddressOf()));

		if constexpr (DebugBuild)
		{
			char buffer[100];
			unsigned int size = (unsigned int)std::wcstombs(buffer, fileName.c_str(), 100);
			shader->SetPrivateData(WKPDID_D3DDebugObjectName, size, buffer);
		}

		return shader;
	}

	ComPtr<ID3D11ComputeShader> CompileComputeShader(ID3D11Device* device, const std::wstring& fileName, const std::string& function, const std::string& model, const D3D_SHADER_MACRO* defines, ComPtr<ID3D10Blob>& bytecode)
	{
		auto errors = ComPtr<ID3D10Blob>{};
		auto res = D3DCompileFromFile(fileName.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, function.c_str(), model.c_str(), GetShaderFlags(), 0, bytecode.GetAddressOf(), errors.GetAddressOf());
		if (FAILED(res))
		{
			auto* errorObj = errors.Get();
			if (errorObj != nullptr)
			{
				auto error = std::string((char*)errorObj->GetBufferPointer());
				TENLog(error, LogLevel::Error);
				throw std::runtime_error(error);
			}
			else
			{
				TENLog("Error while compiling CS shader: " + TEN::Utils::ToString(fileName.c_str()), LogLevel::Error);
				ThrowIfFailed(res);
			}
		}

		auto shader = ComPtr<ID3D11ComputeShader>{};
		ThrowIfFailed(device->CreateComputeShader(bytecode->GetBufferPointer(), bytecode->GetBufferSize(), nullptr, shader.GetAddressOf()));

		if constexpr (DebugBuild)
		{
			char buffer[100];
			unsigned int size = (unsigned int)std::wcstombs(buffer, fileName.c_str(), 100);
			shader->SetPrivateData(WKPDID_D3DDebugObjectName, size, buffer);
		}

		return shader;
	}
}
