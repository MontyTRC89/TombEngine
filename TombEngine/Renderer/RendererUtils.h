#pragma once

namespace TEN::Renderer::Utils
{
	constexpr [[nodiscard]] unsigned int GetShaderFlags();

	void ThrowIfFailed(const HRESULT& res);
	void ThrowIfFailed(const HRESULT& res, const std::string& info);
	void ThrowIfFailed(const HRESULT& res, const std::wstring& info);

	[[nodiscard]] Microsoft::WRL::ComPtr<ID3D11VertexShader>  CompileVertexShader(ID3D11Device* device, const std::wstring& fileName, const std::string& function, const std::string& model, const D3D_SHADER_MACRO* defines, Microsoft::WRL::ComPtr<ID3D10Blob>& bytecode);
	[[nodiscard]] Microsoft::WRL::ComPtr<ID3D11PixelShader>	  CompilePixelShader(ID3D11Device* device, const std::wstring& fileName, const std::string& function, const std::string& model, const D3D_SHADER_MACRO* defines, Microsoft::WRL::ComPtr<ID3D10Blob>& bytecode);
	[[nodiscard]] Microsoft::WRL::ComPtr<ID3D11ComputeShader> CompileComputeShader(ID3D11Device* device, const std::wstring& fileName, const std::string& function, const std::string& model, const D3D_SHADER_MACRO* defines, Microsoft::WRL::ComPtr<ID3D10Blob>& bytecode);
}
