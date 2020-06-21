#pragma once
#include <winerror.h>
namespace T5M {
	namespace Renderer {
		namespace Utils {
			//throws a std::exception when the result contains a FAILED result
			//In most cases we cannot run the game if some Direct3D operation failed
			void throwIfFailed(const HRESULT& res);
		}
	}
}