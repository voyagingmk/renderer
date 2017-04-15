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
		virtual void beforeSet() = 0;
		virtual void afterSet() = 0;
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
		virtual void beforeSet() override;
		virtual void afterSet() override;
		virtual void set(int x, int y, int r, int g, int b) override;
		virtual void resize(int xres, int yres) override;
	};

#ifdef _SDL_H
	class SDLFilm : public Film {
	public:
		SDL_Texture* texture;
		SDL_PixelFormat* pixelFormat;
		Uint32* pixels;
		SDL_Rect lockRect;
	public:
		// Film Interface
		SDLFilm(int xres, int yres);
		virtual ~SDLFilm() {};
		void beforeSet() override;
		void afterSet() override;
		virtual void set(int x, int y, int r, int g, int b) override;
		virtual void resize(int xres, int yres) override;
	};
#endif

};

#endif
