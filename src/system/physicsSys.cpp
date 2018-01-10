#include "stdafx.h"
#include "system/physicsSys.hpp"
#include "com/quaternion.hpp"
#include "com/miscCom.hpp"
#include "event/shaderEvent.hpp"
#include "event/bufferEvent.hpp"

namespace renderer {
    
    class MyEventListener: public rp3d::EventListener {
        /// Called when a new contact point is found between two bodies that were separated before
        /**
         * @param contact Information about the contact
         */
        virtual void beginContact(const ContactPointInfo& contact) {
            printf("beginContact: %f,%f,%f <->", contact.localPoint1.x , contact.localPoint1.y,  contact.localPoint1.z);
            printf("%f,%f,%f\n", contact.localPoint2.x , contact.localPoint2.y,  contact.localPoint2.z);
        }
        
        /// Called when a new contact point is found between two bodies
        /**
         * @param contact Information about the contact
         */
        virtual void newContact(const ContactPointInfo& contact) {
            //printf("newContact: %f,%f,%f <->", contact.localPoint1.x , contact.localPoint1.y,  contact.localPoint1.z);
            //printf("%f,%f,%f\n", contact.localPoint2.x , contact.localPoint2.y,  contact.localPoint2.z);
            
        }
        
        /// Called at the beginning of an internal tick of the simulation step.
        /// Each time the DynamicsWorld::update() method is called, the physics
        /// engine will do several internal simulation steps. This method is
        /// called at the beginning of each internal simulation step.
        virtual void beginInternalTick() {
            
        }
        
        /// Called at the end of an internal tick of the simulation step.
        /// Each time the DynamicsWorld::update() metho is called, the physics
        /// engine will do several internal simulation steps. This method is
        /// called at the end of each internal simulation step.
        virtual void endInternalTick() {
            
        }
    };
    
    Vector3dF Trans(const rp3d::Vector3& p) {
        return Vector3dF(p.x, p.y, p.z);
    }
    
    rp3d::Vector3 Trans(const Vector3dF& p) {
        return rp3d::Vector3(p.x, p.y, p.z);
    }
    
    QuaternionF Trans(const rp3d::Quaternion& q) {
        return QuaternionF(q.w, q.x, q.y, q.z);
    }
    
    rp3d::Quaternion Trans(const QuaternionF& q) {
        return rp3d::Quaternion(q.x, q.y, q.z, q.s);
    }
    
    const rp3d::Transform MakeTransform(const ComponentHandle<SpatialData> com) {
        return rp3d::Transform(Trans(com->pos), Trans(com->orientation));
    }
    
    void PhysicsSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
        printf("PhysicsSystem init\n");
        evtMgr.on<CreateCollisionShapeEvent>(*this);
        evtMgr.on<ComponentAddedEvent<PhysicsWorld>>(*this);
        evtMgr.on<UpdateSpatialDataEvent>(*this);
        evtMgr.on<DebugDrawCollisionShapeEvent>(*this);
    }
    
    void PhysicsSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {
        auto com = m_objMgr->getSingletonComponent<PhysicsWorld>();
        com->accumulator += dt;
        while (com->accumulator >= com->timestep) {
            com->accumulator -= com->timestep;
            if (com->world.getNbRigidBodies() > 0) {
                com->world.update(com->timestep);
            }
        }
        float interpolationFactor = float(com->accumulator) / com->timestep;
        assert(interpolationFactor >= 0.0f && interpolationFactor <= 1.0f);
        for (Object obj: m_objMgr->entities<DynamicObjTag, ColBodyCom>()) {
            auto com = obj.component<ColBodyCom>();
            ComputeTransform(com, interpolationFactor);
            auto spatialData = obj.component<SpatialData>();
            spatialData->pos = Trans(com->body->getTransform().getPosition());
            spatialData->orientation = Trans(com->body->getTransform().getOrientation());
            m_evtMgr->emit<UpdateSpatialDataEvent>(obj, 1);
        }
    }
    
    void PhysicsSystem::receive(const ComponentAddedEvent<PhysicsWorld>& evt) {
        static MyEventListener listener;
        ComponentHandle<PhysicsWorld> com = evt.component;
        com->world.setEventListener(&listener);
         com->world.setNbIterationsVelocitySolver(15);
         com->world.setNbIterationsPositionSolver(8);
    }
    
    void PhysicsSystem::receive(const DebugDrawCollisionShapeEvent &evt) {
        auto meshSet = m_objMgr->getSingletonComponent<MeshSet>();
        auto spSetCom = m_objMgr->getSingletonComponent<ShaderProgramSet>();
        Shader shader = spSetCom->getShader("wireframe");
        shader.use();
        shader.set1b("instanced", true);
        shader.setMatrix4f("modelMat", Matrix4x4::newIdentity());
        m_evtMgr->emit<UploadCameraToShaderEvent>(evt.objCamera, shader);
        const char * insBufferName = "colShapes";
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        std::vector<Matrix4x4Value> modelMatrixes;
        for (Object obj: m_objMgr->entities<ColBodyCom>()) {
            auto com = obj.component<ColBodyCom>();
            BoxShape* shape = (BoxShape*)com->body->getProxyShapesList()->getCollisionShape();
            auto extent = 2 * shape->getExtent();
            auto pos = com->body->getTransform().getPosition();
            Vector3dF p(pos.x, pos.y, pos.z);
            Vector3dF e(extent.x, extent.y, extent.z);
            auto q = Trans(com->body->getTransform().getOrientation());
            Matrix4x4 modelMat = Translate<Matrix4x4>(p) * q.toMatrix4x4() * Scale<Matrix4x4>(e);
            modelMatrixes.push_back(modelMat.transpose().dataRef());
        }
        m_evtMgr->emit<CreateInstanceBufferEvent>(insBufferName);
        m_evtMgr->emit<UpdateInstanceBufferEvent>(insBufferName,
          modelMatrixes.size(),
          sizeof(Matrix4x4Value),
          &modelMatrixes[0],
          true);
        MeshID meshID = meshSet->getMeshID("wfbox");
        m_evtMgr->emit<BindInstanceBufferEvent>(meshID, 0, insBufferName);
        m_evtMgr->emit<DrawMeshBufferEvent>(meshID, 0);
        m_evtMgr->emit<UnbindInstanceBufferEvent>(meshID, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    void PhysicsSystem::receive(const UpdateSpatialDataEvent &evt) {
        if (evt.flag == 1) {
            return;
        }
        Object obj = evt.obj;
        if (obj.hasComponent<ColBodyCom>()) {
            auto spatialData = obj.component<SpatialData>();
            auto com = obj.addComponent<ColBodyCom>();
            com->body->setTransform(MakeTransform(spatialData));
        }
    }
    
    void PhysicsSystem::receive(const CreateCollisionShapeEvent& evt) {
        Object obj = evt.obj;
        if (obj.hasComponent<ColBodyCom>()) {
            return;
        }
        auto spatialData = obj.component<SpatialData>();
        auto shapeSet = m_objMgr->getSingletonComponent<CollisionShapeSet>();
        auto physicsWorld = m_objMgr->getSingletonComponent<PhysicsWorld>();
        rp3d::RigidBody* body = physicsWorld->world.createRigidBody(MakeTransform(spatialData));
        auto com = obj.addComponent<ColBodyCom>();
        com->body = body;
        if (obj.hasComponent<StaticObjTag>()) {
            body->setType(rp3d::BodyType::STATIC);
        } else if (obj.hasComponent<DynamicObjTag>()) {
            body->setType(rp3d::BodyType::DYNAMIC);
        }
        rp3d::Material& material = body->getMaterial();
        material.setBounciness(rp3d::decimal(evt.bounciness));
        material.setFrictionCoefficient(rp3d::decimal(evt.friction));
        
        auto meshSet = m_objMgr->getSingletonComponent<MeshSet>();
        auto meshRef = obj.component<MeshRef>();
        Mesh& mesh = meshSet->getMesh(meshRef->meshID);
        for (size_t i = 0; i < mesh.meshes.size(); i++) {
            SubMesh& subMesh = mesh.meshes[i];
            CollisionShapeID shapeID = meshRef->meshID * 10000 + i;
            if (!shapeSet->hasShape(shapeID)) {
                BBox bound = subMesh.Bound();
                auto boxShape = GetPool<rp3d::BoxShape>()->newElement(rp3d::Vector3(bound.Extent(0) / 2, bound.Extent(1) / 2, bound.Extent(2) / 2));
                shapeSet->shapeDict[shapeID] = {rp3d::Transform(Trans(bound.Center()),
                                                                 rp3d::Quaternion::identity()), boxShape};
            }
            ShapeInfo& info = shapeSet->shapeDict[shapeID];
            auto boxShape = shapeSet->getShape(shapeID);
            body->addCollisionShape(info.shape, info.transform, 1.0f);
        }
        /*
        auto c = bound.Center();
        auto t = body->getTransform();
        auto p = t.getPosition();
        auto o = t.getOrientation();
        
        auto t2 = proxyShape->getLocalToBodyTransform();
        auto p2 = t2.getPosition();
        auto o2 = t2.getOrientation();
   
        int i = 0;
        i+=1;*/
    
    }
    
    // Compute the new transform matrix
    void PhysicsSystem::ComputeTransform(ComponentHandle<ColBodyCom> com, float interpolationFactor) {

        // Get the transform of the rigid body
        rp3d::Transform transform = com->body->getTransform();
        
        // Interpolate the transform between the previous one and the new one
        rp3d::Transform interpolatedTransform =
            rp3d::Transform::interpolateTransforms(com->prevTransform,
                                                   transform,
                                                   interpolationFactor);
        com->prevTransform = transform;
    }

}
