#ifndef RENDERER_EVENT_TEXTURE_HPP
#define RENDERER_EVENT_TEXTURE_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"

namespace renderer {

	class LoadTextureEvent : public ecs::Event<LoadTextureEvent> {
	public:
		LoadTextureEvent(std::string &dirpath,
			const char *filename,
			std::string &aliasname,
			bool hasAlpha = false, bool toLinear = true) :
			dirpath(dirpath),
			filename(filename),
			aliasname(aliasname),
			hasAlpha(hasAlpha),
			toLinear(toLinear)
		{}
		std::string dirpath;
		const char *filename;
		std::string aliasname;
		bool hasAlpha;
		bool toLinear;
	};

	class DestroyTextureEvent : public ecs::Event<DestroyTextureEvent> {
	public:
		DestroyTextureEvent(std::string& aliasname):
			aliasname(aliasname)
		{}
		std::string aliasname;
	};

	class ActiveTextureEvent : public ecs::Event<ActiveTextureEvent> {
	public:
		ActiveTextureEvent(uint32_t idx, std::string& aliasname) :
			idx(idx),
			aliasname(aliasname)
		{}
		uint32_t idx;
		std::string aliasname;
	};
    
    class DeactiveTextureEvent : public ecs::Event<DeactiveTextureEvent> {
    public:
        DeactiveTextureEvent(uint32_t idx) :
        idx(idx)
        {}
        uint32_t idx;
    };

}

#endif
