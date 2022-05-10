#pragma once
#include "RendererDisplayMode.h"
#include <string>
#include <vector>

namespace TEN::Renderer
{
	struct RendererVideoAdapter
	{
		std::string Name;
		int Index;
		std::vector<RendererDisplayMode> DisplayModes;
	};
}