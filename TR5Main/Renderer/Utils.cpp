#include "framework.h"
#include "Utils.h"
#include <winerror.h>

void T5M::Renderer::Utils::throwIfFailed(const HRESULT& res)
{
	if (FAILED(res))
		throw std::exception("An error occured!");
}
