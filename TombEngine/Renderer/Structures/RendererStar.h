#pragma once

#include "Renderer/RendererEnums.h"

namespace TEN::Renderer::Structures
{
	struct RendererStar
	{
		Vector3 Direction  = Vector3::Zero;
		Vector3 Color	   = Vector3::Zero;
		float	Blinking   = 0.0f;
		float	Scale	   = 0.0f;
		float	Extinction = 0.0f;
	};
}