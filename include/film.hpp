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
		virtual ~Film() {};
		virtual void set(int x, int y, int r, int g, int b)= 0;
		virtual void resize(int xres, int yres) = 0;
		inline int width() { return xResolution; }
		inline int height() { return yResolution; }
	};

	class ImageFilm: public Film {
	public:
		typedef cil::CImg<unsigned char> Image;
		Image* img;
	public:
		// Film Interface
		ImageFilm(int xres, int yres, Image* image): Film(xres, yres),img(image) {}
		virtual ~ImageFilm() {};
		virtual void set(int x, int y, int r, int g, int b);
		virtual void resize(int xres, int yres);
	};

	class SDLFilm : public Film {
	public:
		SDL_Surface* img;
	public:
		// Film Interface
		SDLFilm(int xres, int yres);
		virtual ~SDLFilm() {};
		virtual void set(int x, int y, int r, int g, int b);
		virtual void resize(int xres, int yres);
	};
};

#endif
