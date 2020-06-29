#pragma once
namespace T5M::Renderer {
	class RenderView {
	public:
		virtual void prepareFrame() = 0;
		virtual void drawFrame() = 0;
		virtual void finishFrame() = 0;
	};
}
