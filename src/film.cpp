#include "stdafx.h"
#include "film.hpp"
#include "profiler.hpp"

namespace renderer {
	void ImageFilm::beforeSet() {
	}

	void ImageFilm::afterSet() {
	}
	
	void ImageFilm::set(int x, int y, int r, int g, int b) {
		img->atXYZC(x, y, 0, 0) = r;
		img->atXYZC(x, y, 0, 1) = g;
		img->atXYZC(x, y, 0, 2) = b;
	}
	void ImageFilm::resize(int w, int h) {
		img->resize(w, h, 1,3);
	}

#ifdef _SDL_H

	SDLFilm::SDLFilm(int xres, int yres):
		Film(xres, yres),
		pixels(nullptr),
		texture(nullptr),
		pixelFormat(nullptr)
	{
	}

	void SDLFilm::beforeSet() {
		if (!texture)
			return;
		int w, h;
		Uint32 format;
		int pitch;
		SDL_QueryTexture(texture, &format, nullptr, &w, &h);
		if (SDL_LockTexture(texture, &lockRect, (void**)&pixels, &pitch)) {
			checkSDLError(__LINE__);
			return;
		}
		pixelFormat = SDL_AllocFormat(format);
	}

	void SDLFilm::afterSet() {
		if (!texture)
			return; 
		SDL_FreeFormat(pixelFormat);
		pixelFormat = nullptr;
		pixels = nullptr;
		SDL_UnlockTexture(texture);
	}

	void SDLFilm::set(int x, int y, int r, int g, int b) {
		if (!texture || !pixelFormat || !pixels)
			return;
		Uint32 color = SDL_MapRGBA(pixelFormat, r, g, b, 255);
		Uint32 pixelPosition = x - lockRect.x;
		pixels[pixelPosition] = color;
	}

	void SDLFilm::resize(int w, int h) {
	}

#endif

}


