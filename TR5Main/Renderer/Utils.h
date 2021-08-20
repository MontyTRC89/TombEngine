#pragma once
#include <winerror.h>
#include <string>
#include <wrl/client.h>
namespace ten {
	namespace renderer {
		namespace Utils {
			void throwIfFailed(const HRESULT& res) noexcept;
			[[nodiscard]] Microsoft::WRL::ComPtr<ID3D11VertexShader> compileVertexShader(ID3D11Device* device, const std::wstring& fileName, const std::string& function, const std::string& model, const D3D_SHADER_MACRO* defines, Microsoft::WRL::ComPtr<ID3D10Blob>& bytecode);
			constexpr [[nodiscard]] UINT GetShaderFlags();
			[[nodiscard]] Microsoft::WRL::ComPtr<ID3D11PixelShader> compilePixelShader(ID3D11Device* device, const std::wstring& fileName, const std::string& function, const std::string& model, const D3D_SHADER_MACRO* defines, Microsoft::WRL::ComPtr<ID3D10Blob>& bytecode);

		}
	}
}