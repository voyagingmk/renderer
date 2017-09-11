#ifndef RENDERER_EVENT_SHADER_HPP
#define RENDERER_EVENT_SHADER_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/shader.hpp"

namespace renderer {

	class LoadShaderEvent {
	public:
		LoadShaderEvent(ecs::Object obj,
			std::string &dirpath,
			ShaderFileNames names,
			std::string &aliasname):
			obj(obj),
			dirpath(dirpath),
			names(names),
			aliasname(aliasname)
		{}
		ecs::Object obj;
		std::string dirpath;
		ShaderFileNames names;
		std::string aliasname;
        ShaderType type;
    };

}

#endif
