#include "stdafx.h"
#include "parser.hpp"
#include "tiny_obj_loader.h"
#include "transform.hpp"
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


void checkSDLError(int line = -1)
{
#ifndef NDEBUG
	const char *error = SDL_GetError();
	if (*error != '\0')
	{
		printf("SDL Error: %s\n", error);
		if (line != -1)
			printf(" + line: %i\n", line);
		SDL_ClearError();
	}
#endif
}
int main(int argc, char *argv[]) {
	SDL_Window *win = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Texture *bitmapTex = NULL;
	int posX = 100, posY = 100, width = 400, height = 400;

	SDL_Init(SDL_INIT_VIDEO);

	win = SDL_CreateWindow("Renderer", posX, posY, width, height, 0);

	renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

	SDLFilm film(width, height);
	Parser parser;
	parser.parseFromFile("config.json", &film);
	bitmapTex = SDL_CreateTextureFromSurface(renderer, film.img);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, bitmapTex, NULL, NULL);
	SDL_RenderPresent(renderer);
	while (1) {
		SDL_Event e;
		if (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				break;
			}
		}
	}
	SDL_DestroyTexture(bitmapTex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(win);

	SDL_Quit();

	return 0;
}
/* Our program's entry point */
//https://www.opengl.org/wiki/Tutorial1:_Creating_a_Cross_Platform_OpenGL_3.2_Context_in_SDL_(C_/_SDL)
int main3(int argc, char *argv[])
{
	SDL_Window *mainwindow; /* Our window handle */
	SDL_GLContext maincontext; /* Our opengl context handle */

	if (SDL_Init(SDL_INIT_VIDEO) < 0) /* Initialize SDL's Video subsystem */
		sdldie("Unable to initialize SDL"); /* Or die on error */

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	/* Turn on double buffering with a 24bit Z buffer.
	* You may need to change this to 16 or 32 for your system */
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	/* Create our window centered at 512x512 resolution */
	mainwindow = SDL_CreateWindow(PROGRAM_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		320, 240, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!mainwindow) /* Die if creation failed */
		sdldie("Unable to create window");

	checkSDLError(__LINE__);

	/* Create our opengl context and attach it to our window */
	maincontext = SDL_GL_CreateContext(mainwindow);
	checkSDLError(__LINE__);

	SDL_Renderer *renderer = SDL_CreateRenderer(mainwindow, -1, SDL_RENDERER_ACCELERATED);
	SDL_Surface *bitmapSurface = SDL_LoadBMP("hello.bmp");
	SDL_Texture *bitmapTex = SDL_CreateTextureFromSurface(renderer, bitmapSurface);
	SDL_FreeSurface(bitmapSurface);

	/* This makes our buffer swap syncronized with the monitor's vertical refresh */
	SDL_GL_SetSwapInterval(1);

	/* Clear our buffer with a red background */
	//glClearColor(1.0, 0.0, 0.0, 1.0);
	//glClear(GL_COLOR_BUFFER_BIT);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, bitmapTex, NULL, NULL);
	SDL_RenderPresent(renderer);
	/* Swap our back buffer to the front */
	SDL_GL_SwapWindow(mainwindow);
	/* Wait 2 seconds */
	
	SDL_Delay(2000);

	/* Delete our opengl context, destroy our window, and shutdown SDL */
	//SDL_GL_DeleteContext(maincontext);
	//SDL_DestroyWindow(mainwindow);
	//SDL_Quit();

	return 0;
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