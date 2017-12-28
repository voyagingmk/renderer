#ifndef RENDERER_COM_MESH_HPP
#define RENDERER_COM_MESH_HPP

#include "base.hpp"
#include "bbox.hpp"
#include "vertex.hpp"
#include "materialCom.hpp"


namespace renderer {
  	/*
	class Mesh;
	
	class Triangle : public Shape {
	public:
		Mesh* mesh;
		int tri_idx;
		int indexes[3];
	public:
		Triangle(Mesh* m):mesh(m), indexes{ 0,0,0 } {};
		~Triangle();
		Triangle(const Triangle&);
		Triangle operator = (const Triangle&);
		virtual void Init() override;
		virtual int Intersect(Ray&, IntersectResult*) override;		
		virtual BBox Bound() const override; 
		virtual BBox WorldBound() const override;
	};

	class Mesh: public Shape {
	public:
		Vertices vertices;
		UIntArray indexes;
		BBox bbox;
		int face = 0;
		bool reverse = false;
	public:
		Mesh() {}
		Mesh(Vertices& v, UIntArray& i);
		Mesh(const Mesh&);
		~Mesh();
		Mesh operator = (const Mesh&);
		virtual void Init() override;
		virtual int Intersect(Ray&, IntersectResult*) override;
		void initBound();
		void initVertexNormals();
		virtual BBox Bound() const override;
		virtual BBox WorldBound() const override;
	};*/

	class Triangle {
	public:
		Vertex v0, v1, v2;
	public:
		Triangle(Vertex& v0, Vertex& v1, Vertex& v2) :
			v0(v0),
			v1(v1),
			v2(v2)
		{};
		BBox Bound() const {
			const Vector3dF& p0 = v0.position;
			const Vector3dF& p1 = v1.position;
			const Vector3dF& p2 = v2.position;
			return BBox(p0, p1).Union(p2);
		}
	};

	typedef size_t MeshID;
	typedef size_t SubMeshIdx;

	enum class MeshType {
		Tri = 0,
		Lines = 1,
        LineStrip = 2
	};

    
	// SubMesh没有自己的transform
	// 只是Mesh的一个part
    class SubMesh {
        public:
			void InitBound() {
				if (bbox.IsEmpty()) {
					for (int tri_idx = 0, tri_num = indexes.size() / 3; tri_idx < tri_num; tri_idx += 1) {
						uint32_t i0 = indexes[tri_idx * 3];
						uint32_t i1 = indexes[tri_idx * 3 + 1];
						uint32_t i2 = indexes[tri_idx * 3 + 2];
						Triangle tri(vertices[i0], vertices[i1], vertices[i2]);
						bbox = Union(bbox, tri.Bound());
					}
				}
			}
			BBox Bound() {
				return bbox;
			}
			Vertices vertices;
		    UIntArray indexes;
			BBox bbox;
			MeshType meshType = MeshType::Tri;
    };

	// SubMesh集合
	// 处理场景管理，不需要深入到SubMesh
	// 处理渲染队列，需要深入SubMesh，因为不同SubMesh材质不一样
    class Mesh {
        public:
			void InitBound() {
				if (bbox.IsEmpty()) {
					for (SubMesh& subMesh: meshes) {
						bbox = Union(bbox, subMesh.Bound());
					}
				}
			}
			BBox Bound() {
				return bbox;
			}
            std::vector<SubMesh> meshes;
			std::vector<MaterialSettingID> settingIDs;
			BBox bbox;
    };

	// 场景可渲染物体，必须加MeshRef
	// 通过MeshRef从而可以实现Mesh共用
	// MeshRef还可以对Mesh做自定义设置，如SubMesh材质绑定
	struct MeshRef {
		MeshRef(MeshID id) :
			meshID(id),
			meshName("")
		{}
		MeshRef(std::string meshName) :
			meshID(0),
			meshName(meshName)
		{}
		MeshID meshID;
		std::string meshName;
		std::map<SubMeshIdx, MaterialSettingID> customSettingIDDict;
	};

	class MeshSet {
	public:
		MeshSet() :
			idCount(0) {}
		MeshID newMeshID() { return ++idCount; }
		Mesh& newMesh(std::string name, MeshID& meshID) {
			meshID = newMeshID();
			meshDict.insert({ meshID, Mesh() });
			alias2id[name] = meshID;
			return meshDict[meshID];
		}
		Mesh& getMesh(MeshRef& ref) {
			MeshID meshID = ref.meshID | alias2id[ref.meshName];
			return meshDict[meshID];
        }
        Mesh& getMesh(MeshID meshID) {
            return meshDict[meshID];
        }
        Mesh& getMesh(std::string meshName) {
            return meshDict[alias2id[meshName]];
        }
        MeshID getMeshID(std::string meshName) {
            return alias2id[meshName];
        }
		std::map<MeshID, Mesh> meshDict;
		std::map<std::string, MeshID> alias2id;
		MeshID idCount;
	};
};

#endif
