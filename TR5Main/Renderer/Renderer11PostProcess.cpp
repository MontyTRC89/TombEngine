#include "framework.h"
#include "Renderer/Renderer11.h"

namespace TEN::Renderer
{
	void Renderer11::fadeIn()
	{
		m_fadeStatus = RENDERER_FADE_STATUS::FADE_IN;
		m_fadeFactor = 0.0f;
	}

	void Renderer11::fadeOut() {
		m_fadeStatus = RENDERER_FADE_STATUS::FADE_OUT;
		m_fadeFactor = 1.0f;
	}
}
