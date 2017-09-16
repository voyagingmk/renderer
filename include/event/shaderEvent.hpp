#ifndef RENDERER_EVENT_SHADER_HPP
#define RENDERER_EVENT_SHADER_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/shader.hpp"

namespace renderer {

	class LoadShaderEvent : public ecs::Event<LoadShaderEvent> {
	public:
		LoadShaderEvent(std::string &dirpath,
			ShaderFileNames names,
			std::string &aliasname):
			dirpath(dirpath),
			names(names),
			aliasname(aliasname)
		{}
		std::string dirpath;
		ShaderFileNames names;
		std::string aliasname;
        ShaderType type;
    };

	class UploadMatrixToShaderEvent : public ecs::Event<UploadMatrixToShaderEvent> {
		public:
			UploadMatrixToShaderEvent(ecs::Object obj, Shader shader):
				obj(obj),
				shader(shader)
			{}
			ecs::Object obj;
			Shader shader;
	};

	class UploadCameraToShaderEvent : public ecs::Event<UploadCameraToShaderEvent> {
	public:
		UploadCameraToShaderEvent(ecs::Object objCamera, Shader shader) :
			objCamera(objCamera),
			shader(shader)
		{}
		ecs::Object objCamera;
		Shader shader;
	};
};

#endif
