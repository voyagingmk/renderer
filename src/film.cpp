#include "stdafx.h"
#include "film.hpp"

namespace renderer {
	void ImageFilm::set(int x, int y, int r, int g, int b) {	
		img->atXYZC(x, y, 0, 0) = r;
		img->atXYZC(x, y, 0, 1) = g;
		img->atXYZC(x, y, 0, 2) = b;
	}
	void ImageFilm::resize(int w, int h) {
		img->resize(w, h, 1,3);
	}

	SDLFilm::SDLFilm(int xres, int yres):Film(xres, yres) {
		img = SDL_CreateRGBSurface(0, xres, yres, 24, 0, 0, 0, 0);
	}

	void SDLFilm::set(int x, int y, int r, int g, int b) {
		Uint32 color = SDL_MapRGB(img->format, r, g, b);
		SDL_Rect rect = { x, y, 1, 1 };
		int ret =  SDL_FillRect(img, &rect, color);
	}

	void SDLFilm::resize(int w, int h) {
		SDL_FreeSurface(img);
		img = SDL_CreateRGBSurface(0, w, h, 24, 0, 0, 0, 0);
	}
}

