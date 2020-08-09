#pragma once
#include <winerror.h>
#include <string>
#include <wrl/client.h>
namespace T5M {
	namespace Renderer {
		namespace Utils {
			void throwIfFailed(const HRESULT& res) noexcept;
			[[nodiscard]]Microsoft::WRL::ComPtr<ID3D11VertexShader> compileVertexShader(ID3D11Device* device, const std::wstring& fileName, const std::string& function, const std::string& model, const D3D_SHADER_MACRO* defines, Microsoft::WRL::ComPtr<ID3D10Blob>& bytecode) noexcept;
			[[nodiscard]]UINT GetShaderFlags() noexcept;
			[[nodiscard]]Microsoft::WRL::ComPtr<ID3D11PixelShader> compilePixelShader(ID3D11Device* device, const std::wstring& fileName, const std::string& function, const std::string& model, const D3D_SHADER_MACRO* defines, Microsoft::WRL::ComPtr<ID3D10Blob>& bytecode) noexcept;
			template<typename T>
			void setName(T* object, const std::string& name)
			{
				ID3D11DeviceChild* childPtr = nullptr;
				object->QueryInterface<ID3D11DeviceChild>(&childPtr);
				childPtr->SetPrivateData(WKPDID_D3DDebugObjectName, name.length(), name.c_str());
			};
		}
	}
}