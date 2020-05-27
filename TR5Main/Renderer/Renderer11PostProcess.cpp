#include "framework.h"
#include "Renderer11.h"

void Renderer11::EnableCinematicBars(bool value)
{
	m_enableCinematicBars = value;
}

void Renderer11::FadeIn()
{
	m_fadeStatus = RENDERER_FADE_STATUS::FADE_IN;
	m_fadeFactor = 0.0f;
}

void Renderer11::FadeOut()
{
	m_fadeStatus = RENDERER_FADE_STATUS::FADE_OUT;
	m_fadeFactor = 1.0f;
}