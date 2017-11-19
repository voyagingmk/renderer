#ifndef RENDERER_EVENT_TEXTURE_HPP
#define RENDERER_EVENT_TEXTURE_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/shader.hpp"
#include "com/bufferCom.hpp"

namespace renderer {

	
	class LoadCubemapEvent : public ecs::Event<LoadCubemapEvent> {
	public:
		LoadCubemapEvent(const std::string &dirpath,
			std::vector<std::string> filenames,
			const std::string &aliasname,
			size_t channels) :
			dirpath(dirpath),
			filenames(filenames),
			aliasname(aliasname),
			channels(channels)
		{}
		const std::string& dirpath;
		std::vector<std::string> filenames;
		const std::string& aliasname;
		size_t channels;
	};
    
    class CreateDepthTextureEvent : public ecs::Event<CreateDepthTextureEvent> {
    public:
        CreateDepthTextureEvent(const std::string& aliasname,
                                DepthTexType dtType,
                                size_t width,
                                size_t height) :
            aliasname(aliasname),
            dtType(dtType),
            width(width),
            height(height)
        {}
		const std::string& aliasname;
        DepthTexType dtType;
        size_t width;
        size_t height;
    };

	class LoadTextureEvent : public ecs::Event<LoadTextureEvent> {
	public:
		LoadTextureEvent(const std::string& dirpath,
			const std::string& filename,
			const std::string& aliasname,
			size_t channels, bool toLinear = true) :
			dirpath(dirpath),
			filename(filename),
			aliasname(aliasname),
			channels(channels),
			toLinear(toLinear)
		{}
		const std::string& dirpath;
		const std::string& filename;
		const std::string& aliasname;
		size_t channels;
		bool toLinear;
	};

    
    class CreateNoiseTextureEvent : public ecs::Event<CreateNoiseTextureEvent> {
    public:
        CreateNoiseTextureEvent(const std::string aliasname):
            aliasname(aliasname)
        {}
		const std::string aliasname;
    };
    
    
	class DestroyTextureEvent : public ecs::Event<DestroyTextureEvent> {
	public:
		DestroyTextureEvent(const std::string aliasname):
			aliasname(aliasname)
		{}
		const std::string aliasname;
	};

	class ActiveTextureEvent : public ecs::Event<ActiveTextureEvent> {
	public:
        ActiveTextureEvent(Shader shader,
						   const char* sample2DName,
                           uint32_t idx, const char* aliasname) :
            shader(shader),
            sample2DName(sample2DName),
			idx(idx),
			aliasname(aliasname)
        {}
        Shader shader;
		const char* sample2DName;
		uint32_t idx;
		const char* aliasname;
	};

	
	class ActiveTextureByIDEvent : public ecs::Event<ActiveTextureByIDEvent> {
	public:
        ActiveTextureByIDEvent(Shader shader,
				const char* sample2DName,
                uint32_t idx,
                TexID texID) :
			shader(shader),
            sample2DName(sample2DName),
            idx(idx),
			texID(texID)
		{}
        ActiveTextureByIDEvent(Shader shader,
							   const char* sample2DName,
                               uint32_t idx,
                               TexRef texRef) :
        shader(shader),
        sample2DName(sample2DName),
        idx(idx),
        texRef(texRef),
        texID(0)
        {}
        Shader shader;
		const char* sample2DName;
		uint32_t idx;
        TexRef texRef;
		TexID texID;
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
