#ifndef RENDERER_RENDERMODE_HPP
#define RENDERER_RENDERMODE_HPP

#include "base.hpp"

enum class RenderModeEnum
{
	Normal = 1,
	DepthMap = 2
};

struct RenderMode {
	RenderModeEnum mode;
};

#endif