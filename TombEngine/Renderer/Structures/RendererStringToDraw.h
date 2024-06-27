#pragma once

namespace TEN::Renderer::Structures
{
	struct RendererStringToDraw
	{
		float X;
		float Y;
		int Flags;
		std::wstring String;
		Vector3 Color;
		float Scale;
	};
}
