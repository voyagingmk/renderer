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

	SDLFilm::SDLFilm(int xres, int yres):
		Film(xres, yres),
		pixels(nullptr),
		texture(nullptr),
		pixelFormat(nullptr)
	{
		img = SDL_CreateRGBSurface(0, xres, yres, 24, 0, 0, 0, 0);
	}

	void SDLFilm::beforeSet() {
		int w, h;
		Uint32 format;
		SDL_QueryTexture(texture, &format, nullptr, &w, &h);
		if (SDL_LockTexture(texture, &lockRect, (void**)&pixels, &pitch)) {
			checkSDLError(__LINE__);
			return;
		}
		pixelFormat = SDL_AllocFormat(format);
	}

	void SDLFilm::afterSet() {
		SDL_FreeFormat(pixelFormat);
		pixelFormat = nullptr;
		pixels = nullptr;
		pitch = 0;
		SDL_UnlockTexture(texture);
	}

	void SDLFilm::set(int x, int y, int r, int g, int b) {
		if (!pixelFormat || !pixels)
			return;
		Uint32 color = SDL_MapRGBA(pixelFormat, r, g, b, 255);
		Uint32 pixelPosition = x - lockRect.x;
		pixels[pixelPosition] = color;
		//SDL_Rect rect = { x, y, 1, 1 };
		//int ret = SDL_FillRect(img, &rect, color);
	}

	void SDLFilm::resize(int w, int h) {
		SDL_FreeSurface(img);
		img = SDL_CreateRGBSurface(0, w, h, 24, 0, 0, 0, 0);
	}
}

