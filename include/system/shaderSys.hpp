#ifndef RENDERER_SYSTEM_SHADER_HPP
#define RENDERER_SYSTEM_SHADER_HPP

#include "base.hpp"
#include "../ecs/ecs.hpp"
#include "event/shaderEvent.hpp"


using namespace ecs;

namespace renderer {
	class ShaderSystem : public System<ShaderSystem>, public Receiver<ShaderSystem>
	{
    public:
        void init(ObjectManager &objMgr, EventManager &evtMgr) override;
        
        void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override {}
        
        void receive(const LoadShaderEvent& evt);

		void receive(const UploadMatrixToShaderEvent& evt);

		void receive(const UploadCameraToShaderEvent& evt);
        
    private:

        ShaderProgramHDL createShaderProgram(SPHDLList& spHDLs, ShaderHDLSet shaderHDLSet);
        
        ShaderHDL loadShaderFromFile(ShaderType type, const char* path);
        
        ShaderHDL loadShaderFromStr(ShaderType type, const char* source);
        
        void deleteShader(ShaderHDL shaderHDL);
    
        bool isShader(ShaderHDL hdl);
    };

};

#endif
