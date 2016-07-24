#include "stdafx.h"
#include "parser.hpp"
#include "tiny_obj_loader.h"
#include "matrix.hpp"
#include "geometry.hpp"
#include "curve.hpp"
#include "film.hpp"
#include "draw.hpp"
#include "profiler.hpp"

using namespace renderer;
using json = nlohmann::json;


void SDLExit(const char *msg)
{
	printf("%s: %s\n", msg, SDL_GetError());
	SDL_Quit();
	exit(1);
}

//https://www.opengl.org/wiki/Tutorial1:_Creating_a_Cross_Platform_OpenGL_3.2_Context_in_SDL_(C_/_SDL)

int main(int argc, char *argv[])
{
	SDL_Window * win = nullptr;
	SDL_Renderer * rendererSDL = nullptr;
	int width = 512, height = 512;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		SDLExit("Unable to initialize SDL");
	checkSDLError(__LINE__);//on Win7 would cause a ERROR about SHCore.dll, just ignore it.
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	checkSDLError(__LINE__);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	checkSDLError(__LINE__);

	json config = readJson("config.json");
	width = config["width"], height = config["height"];
	std::string title = config["title"];

	win = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!win)
		SDLExit("Unable to create window");

	SDL_GLContext glContext = SDL_GL_CreateContext(win);
	checkSDLError(__LINE__);

	SDLFilm film(width, height);
	SceneParser parser;
	SceneDesc desc = parser.parse(config);
	desc.setFilm(&film);
	desc.init();
	SDL_SetWindowSize(win, desc.width, desc.height);
	rendererSDL = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	SDL_Texture* texture = SDL_CreateTexture(rendererSDL, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, desc.width, desc.height);
	checkSDLError(__LINE__);
	film.texture = texture;
	int p = 0;
	double angle = 0;
	Renderer renderer;
	SDL_Point screenCenter = { width / 2, height / 2 };
	SDL_RendererFlip flip = SDL_FLIP_NONE;
	int total = desc.width * desc.height;
	renderer.colorArray = new Color[total];
	renderer.flags = new bool[total];
	for (int i = 0; i < total; i++) {
		renderer.flags[i] = false;
	}
	renderer.asyncRender(desc);
	while (1) {
		SDL_Event e;
		if (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				break;
			}
		}
		int p_new = renderer.getFinishedPixelsCount();
		if (p_new >= total - 1) {
			//printf("render ok");
		}
		if (p_new - p > 0) {

			if (p_new / desc.width != p / desc.width) {
				p_new = (1 + p / desc.width) * desc.width - 1;
			}
			if (p_new - p <= 0) {
				p_new = p + 1;
			}
			SDL_Rect& rect = static_cast<SDLFilm*>(desc.film)->lockRect;
			rect.x = p % desc.width;
			rect.y = p / desc.width;
			rect.w = std::min(p_new - p, desc.width);
			rect.h = 1;
			desc.film->beforeSet();
			for (int i = 0, new_pixels = p_new - p; i < new_pixels; i++) {
				int _p = p + i;
				int x = _p % desc.width, y = _p / desc.width;
				//printf("x y = %d %d \n ", x, y);
				Color& c = renderer.colorArray[_p];
				desc.film->set(x, y, c.rInt(), c.gInt(), c.bInt());
			}
			desc.film->afterSet();
			SDL_Rect updatedRect = rect;
			glClearColor(1.0, 1.0, 1.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT);
			//printf("rect %d,%d,%d,%d\n", rect.x, rect.y, rect.w, rect.h);
			int ret = SDL_RenderCopyEx(rendererSDL, texture, &updatedRect, &updatedRect, angle, &screenCenter, flip);
			if (ret == -1)
				SDLExit("SDL_RenderCopy failed");
			p = p_new + 1;
			SDL_RenderPresent(rendererSDL);
		}
		//SDL_Delay(1);
	}
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(rendererSDL);
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(win);
	SDL_Quit();

	return 0;
}



/*
Render Spline

int main(int argc, char ** argv) {

	Parser parser;
	//parser.parseFromFile("config.json");
	std::vector<Point2dF> points = {
		{-1.0f, 0.0f},
		{-0.5f, 0.5f},
		{0.5f, -0.5f},
		{1.0f, 0.0f}
	};
	Spline spline;
	spline.setOrder(3);
	spline.setCtrlPoints(points);
	spline.initSplineParam();

	cil::CImg<unsigned char> img(500, 500, 1, 3);
	for (int i = 0; i < img.width(); i++)
		for (int j = 0; j < img.width(); j++)
			img.atXYZC(i, j, 0, 0) = img.atXYZC(i, j, 0, 2) = img.atXYZC(i, j, 0, 2) = 0;
	Point2dF result;
	for (float t = 0; t < 1.0f; t += 0.01f) {
		spline.interpolate(&result, t);
		printf("%f,%f\n", result.x, result.y);
		int x = int(((result.x + 1) / 2.f) *500);
		int y = int((1.0f - ((result.y+1) /2.f)) * 500);
		//img.atXYZC(x, y, 0, 0) = 255;
	}
	ImageFilm film(img.width(), img.height(), &img);
	DrawPen pen;
	pen.drawline(&film, 100, 100, 400, 300, Color(255, 0, 0));
	pen.drawline(&film, 0, 100, 400, 300, Color(0, 255, 0));
	img.display("");
	return 0;
}
*/