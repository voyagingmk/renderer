#ifndef RENDERER_SHAPE_HPP
#define RENDERER_SHAPE_HPP

#include "base.hpp"
#include "transform.hpp"
#include "bbox.hpp"
#include "quaternion.hpp"
#include "material.hpp"

namespace renderer {
	class Shape {
		//default Normal = (0, 1, 0)
		//default Position = (0, 0, 0)
	public:
        int id;
        MaterialID matID;
        std::string matAlias;
		Transform4x4* o2w;
        Transform4x4* w2o;
        Vector3dF pos;
        Vector3dF scale;
        Vector3dF rotate;
	public:
		Shape() noexcept:
            matID(0),
            matAlias(""),
            o2w(nullptr),
            w2o(nullptr),
            pos(0.0f, 0.0f, 0.0f),
            scale(1.0f, 1.0f, 1.0f),
            rotate(0.0f, 0.0f, 0.0f)
        {}
        
		virtual ~Shape() noexcept {
			o2w = nullptr;
			w2o = nullptr;
			matID = 0;
            matAlias = "";
		}
        
        void SetPos(Vector3dF p) {
            pos = p;
            UpdateTransform();
        }
        
        void SetRotate(float angle, Axis axis) {
            if(axis == Axis::x) {
                rotate.x = angle;
            } else if(axis == Axis::y) {
                rotate.y = angle;
            } else if(axis == Axis::z) {
                rotate.z = angle;
            }
            UpdateTransform();
        }
        
        void SetScale(Vector3dF s) {
            scale = s;
            UpdateTransform();
        }
        
        void UpdateTransform() {
            Matrix4x4 T = Translate<Matrix4x4>({pos.x, pos.y, pos.z});
            Matrix4x4 S = Scale<Matrix4x4>({scale.x, scale.y, scale.z});
            QuaternionF identity = {1.0, 0.0, 0.0, 0.0};
            QuaternionF rotX = QuaternionF::RotateX(rotate.x); // x
            QuaternionF rotY = QuaternionF::RotateY(rotate.y); // y
            QuaternionF rotZ = QuaternionF::RotateZ(rotate.z); // z
            QuaternionF orientation = identity * rotZ * rotY * rotX;
            orientation = orientation.Normalize();
            Matrix4x4 R = orientation.toMatrix4x4();
            o2w->m = T * R * S;
            o2w->mInv = o2w->m.inverse();
        }
    
        void InitIdentityTransform() {
            auto pool_Transform = GetPool<Transform>();
            o2w = pool_Transform->newElement(Matrix4x4::newIdentity(), Matrix4x4::newIdentity());
            w2o = pool_Transform->newElement(o2w->mInv, o2w->m);
        }
        
		virtual void Init() = 0;
		virtual int Intersect(Ray&, IntersectResult*) = 0;
		virtual BBox Bound() const = 0;
		virtual BBox WorldBound() const = 0;
	};

	typedef std::vector<Shape*> Shapes;
}

#endif //RENDERER_SHAPE_HPP
