#include "framework.h"
#include "Utils.h"
#include <winerror.h>
#include <iostream>
#include <wrl/client.h>
#include <d3dcompiler.h>
namespace T5M::Renderer::Utils {
	using std::wstring;
	using std::string;
	using Microsoft::WRL::ComPtr;
	using std::vector;
	void Utils::throwIfFailed(const HRESULT& res) noexcept {
		if(FAILED(res)){
			std::string message = std::system_category().message(res);
			std::cout << message << std::endl;
			throw std::runtime_error("An error occured!");
		}
			
	}

	ComPtr<ID3D11VertexShader> compileVertexShader(ID3D11Device* device, const std::wstring& fileName, const std::string& function, const std::string& model, const D3D_SHADER_MACRO * defines, ComPtr<ID3D10Blob>& bytecode) {
		ComPtr<ID3D10Blob> errors;
		logD("Compiling vertex shader");
		throwIfFailed(D3DCompileFromFile(fileName.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, function.c_str(), model.c_str(), GetShaderFlags(), 0, bytecode.GetAddressOf(),errors.GetAddressOf()));
		ComPtr<ID3D11VertexShader> shader;
		throwIfFailed(device->CreateVertexShader(bytecode->GetBufferPointer(), bytecode->GetBufferSize(), nullptr, shader.GetAddressOf()));
		if constexpr(DebugBuild){
			char buffer[100];
			size_t sz = std::wcstombs(buffer, fileName.c_str(), 100);
			shader->SetPrivateData(WKPDID_D3DDebugObjectName, sz, buffer);
		}
		return shader;
	}
	ComPtr<ID3D11PixelShader> compilePixelShader(ID3D11Device* device, const wstring& fileName, const string& function, const string& model, const D3D_SHADER_MACRO* defines, ComPtr<ID3D10Blob>& bytecode) {
		ComPtr<ID3D10Blob> errors;
		logD("Compiling pixel shader");
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_SKIP_OPTIMIZATION;
		throwIfFailed(D3DCompileFromFile(fileName.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, function.c_str(), model.c_str(), GetShaderFlags(), 0, bytecode.GetAddressOf(), errors.GetAddressOf()));
		ComPtr<ID3D11PixelShader> shader;
		throwIfFailed(device->CreatePixelShader(bytecode->GetBufferPointer(), bytecode->GetBufferSize(), nullptr, shader.GetAddressOf()));
		if constexpr(DebugBuild){
			char buffer[100];
			size_t sz = std::wcstombs(buffer, fileName.c_str(), 100);
			shader->SetPrivateData(WKPDID_D3DDebugObjectName, sz, buffer);
		}
		return shader;
	}
	

	constexpr UINT Utils::GetShaderFlags()
	{
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
		if constexpr(DebugBuild){
			flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
		} else{
			flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_IEEE_STRICTNESS;
		}
		return flags;
	}
}
