#ifndef RENDERER_COM_MESH_HPP
#define RENDERER_COM_MESH_HPP

#include "base.hpp"
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


	typedef size_t MeshID;
	typedef size_t SubMeshIdx;

	// SubMeshû���Լ���transform
	// ֻ��Mesh��һ��part
    class SubMesh {
        public:
			SubMesh():
				settingID(0)
			{}
            Vertices vertices;
		    UIntArray indexes;
			MaterialSettingID settingID; // default
    };

	// SubMesh����
	// ��������������Ҫ���뵽SubMesh
	// ������Ⱦ���У���Ҫ����SubMesh����Ϊ��ͬSubMesh���ʲ�һ��
    class Mesh {
        public:
			Mesh() {}
            ~Mesh() {
                meshes.clear();
            }
			Mesh(std::vector<SubMesh>& m)
            {
                std::copy(m.begin(), m.end(),
                   std::back_inserter(meshes));
            }
            std::vector<SubMesh> meshes;
    };

	// ��������Ⱦ���壬�����MeshRef
	// ͨ��MeshRef�Ӷ�����ʵ��Mesh����
	// MeshRef�����Զ�Mesh���Զ������ã���SubMesh���ʰ�
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
		std::map<SubMeshIdx, MaterialSettingID> settingIDs;
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
			return meshDict[ref.meshID];
		}
		std::map<MeshID, Mesh> meshDict;
		std::map<std::string, MeshID> alias2id;
		MeshID idCount;
	};
};

#endif
