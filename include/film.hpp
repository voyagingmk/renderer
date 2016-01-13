#ifndef RENDERER_FILM_HPP
#define RENDERER_FILM_HPP

#include "base.hpp"

namespace renderer {
	class Film {
	public:
		// Film Public Data
		const int xResolution, yResolution;
	public:
		// Film Interface
		Film(int xres, int yres)
			: xResolution(xres), yResolution(yres) { }
		virtual ~Film();
		virtual void set(int x, int y, int r, int g, int b) = 0;
	};

	class ImageFilm: Film {
	public:
		typedef cil::CImg<unsigned char> Image;
		Image img;
	public:
		// Film Interface
		ImageFilm(int xres, int yres, Image& image): Film(xres, yres),img(image) {}
		virtual ~ImageFilm();
		virtual void set(int x, int y, int r, int g, int b);
	};
};

#endif
