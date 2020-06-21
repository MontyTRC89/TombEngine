#pragma once
#include <winerror.h>
namespace T5M {
	namespace Renderer {
		namespace Utils {
			void throwIfFailed(const HRESULT& res);
		}
	}
}