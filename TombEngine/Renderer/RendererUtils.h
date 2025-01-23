#pragma once

namespace TEN::Renderer::Utils
{
	constexpr [[nodiscard]] unsigned int GetShaderFlags();

	void ThrowIfFailed(const HRESULT& res);
	void ThrowIfFailed(const HRESULT& res, const std::string& info);
	void ThrowIfFailed(const HRESULT& res, const std::wstring& info);

	std::wstring GetAssetPath(const wchar_t* fileName);
}
