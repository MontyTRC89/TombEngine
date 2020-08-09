#pragma once
#include <winerror.h>
#include <string>
namespace T5M {
	namespace Renderer {
		namespace Utils {
			//throws a std::exception when the result contains a FAILED result
			//In most cases we cannot run the game if some Direct3D operation failed
			void throwIfFailed(const HRESULT& res) noexcept;
			ID3D11VertexShader* compileVertexShader(ID3D11Device* device, const std::wstring& fileName, const std::string& function, const std::string& model, const D3D_SHADER_MACRO * defines, ID3D10Blob** bytecode);
			UINT GetShaderFlags();
			ID3D11PixelShader* compilePixelShader(ID3D11Device* device, const std::wstring& fileName, const std::string& function, const std::string& model, const D3D_SHADER_MACRO * defines, ID3D10Blob** bytecode) noexcept;
		}
	}
}