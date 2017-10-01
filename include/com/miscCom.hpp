#ifndef RENDERER_COM_MISC_HPP
#define RENDERER_COM_MISC_HPP

#include "base.hpp"

namespace renderer {
    
	struct TagComBase {};

	struct GlobalQuadTag: public TagComBase {};


	struct SkyboxCom {
		SkyboxCom(std::string texName):
			texName(texName)
		{}
		std::string texName;
	};

    struct GlobalSkyboxTag : public TagComBase {};
    
    struct ReceiveLightTag : public TagComBase {};
}

#endif
