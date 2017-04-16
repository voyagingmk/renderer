#include "stdafx.h"
// #include "main_raytrace.hpp"
#include "realtime/shadermgr.hpp"
#include "realtime/context.hpp"



using namespace std;
using namespace renderer;


class MyContext : public RendererContextSDL {
public:
	virtual void onSDLEvent(SDL_Event& e) {
		if (e.type == SDL_QUIT) {
			shouldExit = true;
		}
	}
};


int main(int argc, char *argv[]) {
	MyContext context;
	context.setup(800, 600);
	context.loop();
	return 0;
}
