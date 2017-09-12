#include "stdafx.h"
#include "system/spatialSys.hpp"

using namespace std;

namespace renderer {


	void SpatialSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
        evtMgr.on<UpdateSpatialDataEvent>(*this);
        evtMgr.on<ComponentAddedEvent<SpatialData>>(*this);
    }
    

	void SpatialSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {

	}
    
    void SpatialSystem::receive(const ComponentAddedEvent<SpatialData> &evt) {
        updateSpatialData(evt.m_obj);
    }
    
	void SpatialSystem::receive(const UpdateSpatialDataEvent &evt) {
        updateSpatialData(evt.obj);
    }
    
    void SpatialSystem::updateSpatialData(Object obj) {
        auto com = obj.component<SpatialData>();
        Matrix4x4 T = Translate<Matrix4x4>({com->pos.x, com->pos.y, com->pos.z});
        Matrix4x4 S = Scale<Matrix4x4>({com->scale.x, com->scale.y, com->scale.z});
        com->orientation = com->orientation.Normalize();
        Matrix4x4 R = com->orientation.toMatrix4x4();
		T.debug();
		S.debug();
		R.debug();
        com->o2w = T * R * S;
    }

	void SpatialSystem::receive(const ActiveSpatialDataEvent &evt) {
		auto com = evt.obj.component<SpatialData>();
		Shader& shader = const_cast<Shader&>(evt.shader);
		shader.setMatrix4f("model", com->o2w.m);
		shader.setMatrix4f("normalMat", com->o2w.mInv.transpose());
	}

};
