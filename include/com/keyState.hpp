#ifndef RENDERER_COM_KEYSTATE_HPP
#define RENDERER_COM_KEYSTATE_HPP

#include "base.hpp"

struct KeyState {
	std::map<SDL_Keycode, uint8_t> state;
};

#endif