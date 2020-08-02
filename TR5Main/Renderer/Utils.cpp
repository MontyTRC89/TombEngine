#include "framework.h"
#include "Utils.h"
#include <winerror.h>
namespace T5M::Renderer::Utils {
	using std::wstring;
	using std::string;
	void Utils::throwIfFailed(const HRESULT& res) noexcept {
		if(FAILED(res))
			throw std::exception("An error occured!");
	}

	ID3D11VertexShader* compileVertexShader(ID3D11Device* device, const wstring& fileName, const string& function, const string& model, const D3D_SHADER_MACRO* defines, ID3D10Blob** bytecode) {
		HRESULT res;

		*bytecode = nullptr;
		ID3DBlob* errors = nullptr;
		logD("Compiling vertex shader");
		res = D3DCompileFromFile(fileName.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, function.c_str(), model.c_str(), GetShaderFlags(), 0, bytecode, &errors);
		throwIfFailed(res);
		ID3D11VertexShader* shader = nullptr;
		res = device->CreateVertexShader((*bytecode)->GetBufferPointer(), (*bytecode)->GetBufferSize(), nullptr, &shader);
		throwIfFailed(res);
		return shader;
	}
	ID3D11PixelShader* compilePixelShader(ID3D11Device* device, const wstring& fileName, const string& function, const string& model, const D3D_SHADER_MACRO* defines, ID3D10Blob** bytecode) noexcept {
		HRESULT res;

		*bytecode = nullptr;
		ID3DBlob* errors = nullptr;
		logD("Compiling pixel shader");
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_SKIP_OPTIMIZATION;
		throwIfFailed(D3DCompileFromFile(fileName.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, function.c_str(), model.c_str(), GetShaderFlags(), 0, bytecode, &errors));
		ID3D11PixelShader* shader = nullptr;
		throwIfFailed(device->CreatePixelShader((*bytecode)->GetBufferPointer(), (*bytecode)->GetBufferSize(), nullptr, &shader));
		return shader;
	}
	UINT Utils::GetShaderFlags()
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
