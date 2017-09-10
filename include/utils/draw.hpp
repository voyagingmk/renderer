#ifndef RENDERER_DRAW_HPP
#define RENDERER_DRAW_HPP

#include "base.hpp"
#include "film.hpp"
#include "com/color.hpp"

namespace renderer {
	class DrawPen {
	public:
		void drawline(Film* film, float x0, float y0, float x1, float y1, Color& c) {
			float p1 = y0 - y1, p2 = x1 - x0, p3 = x0 * y1 - x1 * y0;
			auto f = [&](float x, float y) { return p1 * x + p2 * y + p3; };
			float y = y0;
			float d = f(x0 + 1.0f, y0 + 0.5f);
			for (float x = x0; x < x1; x += 1.0f) {
				film->set(x, y, c.r(), c.g(), c.b());
				if (d < 0) {
					y = y + 1.0f;
					d = d + (x1 - x0) + (y0 - y1);
				}
				else {
					d = d + (y0 - y1);
				}
			}
		}
	};
};

#endif
