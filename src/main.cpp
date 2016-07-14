#include "stdafx.h"
#include "parser.hpp"
#include "tiny_obj_loader.h"
#include "matrix.hpp"
#include "geometry.hpp"
#include "curve.hpp"
#include "film.hpp"
#include "draw.hpp"

using namespace renderer;

#define PROGRAM_NAME "Tutorial1"

/* A simple function that prints a message, the error code returned by SDL,
* and quits the application */
void sdldie(const char *msg)
{
	printf("%s: %s\n", msg, SDL_GetError());
	SDL_Quit();
	exit(1);
}





int main3(int argc, char *argv[]) {
	SDL_Window *win = NULL;
	SDL_Renderer *rendererSDL = NULL;
	SDL_Texture *bitmapTex = NULL;
	int width = 512, height = 512;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		sdldie("Unable to initialize SDL");

	checkSDLError(__LINE__);

	using json = nlohmann::json;
	json config = readJson("config.json");
	width = config["width"], height = config["height"];

	win = SDL_CreateWindow("Renderer", 
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		width, height, 0);
	
	if (!win) /* Die if creation failed */
		sdldie("Unable to create window");

	rendererSDL = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

	SDLFilm film(width, height);
	SceneParser parser; 
	SceneDesc desc = parser.parse(config);
	desc.setFilm(&film);
	Renderer renderer;
	renderer.renderScene(desc);
	bitmapTex = SDL_CreateTextureFromSurface(rendererSDL, film.img);

	SDL_RenderClear(rendererSDL);
	SDL_RenderCopy(rendererSDL, bitmapTex, NULL, NULL);
	SDL_RenderPresent(rendererSDL);
	while (1) {
		SDL_Event e;
		if (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				break;
			}
		}
		SDL_RenderPresent(rendererSDL);
		SDL_Delay(30);

	}
	SDL_DestroyTexture(bitmapTex);
	SDL_DestroyRenderer(rendererSDL);
	SDL_DestroyWindow(win);

	SDL_Quit();

	return 0;
}
/* Our program's entry point */
//https://www.opengl.org/wiki/Tutorial1:_Creating_a_Cross_Platform_OpenGL_3.2_Context_in_SDL_(C_/_SDL)
int main(int argc, char *argv[])
{
	SDL_Window *win = NULL;
	SDL_GLContext glContext = NULL; /* Our opengl context handle */
	int width = 512, height = 512;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		sdldie("Unable to initialize SDL");

	checkSDLError(__LINE__);//on Win7 would cause a ERROR about SHCore.dll, just ignore it.

	using json = nlohmann::json;
	json config = readJson("config.json");
	width = config["width"], height = config["height"];

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	checkSDLError(__LINE__);

	/* Turn on double buffering with a 24bit Z buffer.
	* You may need to change this to 16 or 32 for your system */
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	checkSDLError(__LINE__);

	win = SDL_CreateWindow(PROGRAM_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!win)
		sdldie("Unable to create window");

	checkSDLError(__LINE__);

	/* Create our opengl context and attach it to our window */
	glContext = SDL_GL_CreateContext(win);
	checkSDLError(__LINE__);

	SDL_Renderer * rendererSDL = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	
	SDL_Texture *bitmapTex;
	SDLFilm film(width, height);
	SceneParser parser;
	SceneDesc desc = parser.parse(config);
	desc.setFilm(&film);
	Renderer renderer;
	//renderer.renderScene(desc);
	bitmapTex = SDL_CreateTextureFromSurface(rendererSDL, film.img);
	/*
	{
		SDL_Surface *bitmapSurface = SDL_LoadBMP("hello.bmp");
		bitmapTex = SDL_CreateTextureFromSurface(renderer, bitmapSurface);
		SDL_FreeSurface(bitmapSurface);
	}*/
	checkSDLError(__LINE__);
	/* This makes our buffer swap syncronized with the monitor's vertical refresh */
	SDL_GL_SetSwapInterval(1);

	/* Clear our buffer with a red background */
	glClearColor(1.0, 1.0,1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	SDL_SetRenderDrawColor(rendererSDL, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(rendererSDL);
	int ret = SDL_RenderCopy(rendererSDL, bitmapTex, NULL, NULL);
	if (ret == -1)
		sdldie("SDL_RenderCopy failed");
	SDL_RenderPresent(rendererSDL);
	
	/* Swap our back buffer to the front */
	//SDL_GL_SwapWindow(win);

	std::mutex mtx;
	int p = 0;
	renderer.initRenderDesc(desc);
	while (1) {
		SDL_Event e;
		if (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				break;
			}
		}
		renderer.asyncRender(desc, mtx, p);

		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
			SDL_Rect rect;//2row
			rect.x = 0;
			rect.y = p / desc.width;
			rect.w = desc.width;
			rect.h = 2;
			int ret = SDL_UpdateTexture(bitmapTex, &rect, film.img->pixels, 3);
			//printf("ret=%d\n", ret);
			//SDL_Texture * bitmapTex = SDL_CreateTextureFromSurface(rendererSDL, film.img);
			ret = SDL_RenderCopy(rendererSDL, bitmapTex, &rect, &rect);
			//SDL_DestroyTexture(bitmapTex); 
		p += 4;
		SDL_RenderPresent(rendererSDL);
		SDL_Delay(1);

	}
	SDL_DestroyTexture(bitmapTex);
	SDL_DestroyRenderer(rendererSDL);
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(win);
	SDL_Quit();

	return 0;
}

void drawPixel(SDL_Renderer * rendererSDL, Color c, int x, int y) {
	SDL_SetRenderDrawColor(rendererSDL, c.rInt(), c.gInt(), c.bInt(), SDL_ALPHA_OPAQUE);
	SDL_RenderDrawPoint(rendererSDL, x, y);
}

void renderScene(SDLFilm * film) {

}

/*

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