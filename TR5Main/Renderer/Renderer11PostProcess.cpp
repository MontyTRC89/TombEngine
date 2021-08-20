#include "framework.h"
#include "Renderer11.h"
namespace ten::renderer {
	void Renderer11::enableCinematicBars(bool value) {
		m_enableCinematicBars = value;
	}

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
