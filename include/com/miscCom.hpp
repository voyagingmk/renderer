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
    
    struct PointLightTransform {
        float fovy;
        float aspect;
        float n;
        float f;
        Matrix4x4 shadowProj;
        std::vector<Matrix4x4> lightPVs;
        void update(Vector3dF lightPos) {
            shadowProj = Perspective(90.0f, aspect, n, f);
            std::vector<Matrix4x4> lightPVs;
            if (lightPVs.size() == 0) {
                lightPVs.resize(6);
            }
            lightPVs[0] = shadowProj * LookAt(lightPos, lightPos + Vector3dF{1.0, 0.0, 0.0}, {0.0, -1.0, 0.0});  // right
            lightPVs[1] = shadowProj * LookAt(lightPos, lightPos + Vector3dF{-1.0, 0.0, 0.0}, {0.0, -1.0, 0.0}); // left
            lightPVs[2] = shadowProj * LookAt(lightPos, lightPos + Vector3dF{0.0, 1.0, 0.0}, {0.0, 0.0, 1.0});   // top
            lightPVs[3] = shadowProj * LookAt(lightPos, lightPos + Vector3dF{0.0, -1.0, 0.0}, {0.0, 0.0, -1.0}); // bottom
            lightPVs[4] = shadowProj * LookAt(lightPos, lightPos + Vector3dF{0.0, 0.0, 1.0}, {0.0, -1.0, 0.0});  // near
            lightPVs[5] = shadowProj * LookAt(lightPos, lightPos + Vector3dF{0.0, 0.0, -1.0}, {0.0, -1.0, 0.0}); // far
        }
    };

}

#endif
