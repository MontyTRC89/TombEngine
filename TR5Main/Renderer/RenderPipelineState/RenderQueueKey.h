#pragma once
#include <cstdint>
namespace T5M::Renderer::RenderQueue {
	//https://realtimecollisiondetection.net/blog/?p=86
	union RenderQueueKey {
		using KeyType = uint32_t;
		struct KeyDetails {
			KeyType HUD : 1;
			KeyType Translucency : 1;
			KeyType Depth : 24;
			KeyType Material : 3;
			KeyType Pass : 2;
		};
		KeyDetails Details;
		KeyType Value;
	};
}