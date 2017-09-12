#ifndef RENDERER_EVENT_RENDER_HPP
#define RENDERER_EVENT_RENDER_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/color.hpp"

namespace renderer {

	class RenderSceneEvent: public ecs::Event<RenderSceneEvent> {
	public:
		RenderSceneEvent(ecs::Object cameraObj,
			Color clearColor = Color(0.1f, 0.1f, 0.1f, 1.0f),
			uint32_t clearBits = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT):
			cameraObj(cameraObj),
			clearColor(clearColor),
			clearBits(clearBits)
		{}
		ecs::Object cameraObj;
		Color clearColor;
		uint32_t clearBits;
	};

};

#endif
