#ifndef RENDERER_EVENT_TEXTURE_HPP
#define RENDERER_EVENT_TEXTURE_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"

namespace renderer {

	class LoadTextureEvent {
	public:
		LoadTextureEvent(ecs::Object obj,
			std::string &dirpath,
			const char *filename,
			std::string &aliasname,
			bool hasAlpha = false, bool toLinear = true) :
			obj(obj),
			dirpath(dirpath),
			filename(filename),
			aliasname(aliasname),
			hasAlpha(hasAlpha),
			toLinear(toLinear)
		{}
		ecs::Object obj;
		std::string dirpath;
		const char *filename;
		std::string aliasname;
		bool hasAlpha;
		bool toLinear;
	};

	class DestroyTextureEvent {
	public:
		DestroyTextureEvent(ecs::Object obj, std::string& aliasname):
			obj(obj),
			aliasname(aliasname)
		{}
		ecs::Object obj;
		std::string aliasname;
	};

	class ActiveTextureEvent {
	public:
		ActiveTextureEvent(ecs::Object obj, uint32_t idx, std::string& aliasname) :
			obj(obj),
			idx(idx),
			aliasname(aliasname)
		{}
		ecs::Object obj;
		uint32_t idx;
		std::string aliasname;
	};
}

#endif