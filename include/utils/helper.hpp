#ifndef RENDERER_UTILS_HELPER_HPP
#define RENDERER_UTILS_HELPER_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/color.hpp"
#include "com/mesh.hpp"

namespace renderer{

    static Color parseColor(nlohmann::json &c)
    {
        if (c.is_null())
        {
            return Color::White;
        }
        if (c.is_string())
        {
            if (c == "Red")
                return Color::Red;
            else if (c == "White")
                return Color::White;
            else if (c == "Black")
                return Color::Black;
            else if (c == "Green")
                return Color::Green;
            else if (c == "Blue")
                return Color::Blue;
            else
                return Color::White;
        }
        else
        {
            return Color((float)c[0] / 255.0f, (float)c[1] / 255.0f, (float)c[2] / 255.0f);
        }
    }

	static float lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	template<typename T>
	static T random0_1() {
		static std::uniform_real_distribution<T> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
		static std::default_random_engine generator;
		return randomFloats(generator);
	}

	static void generateSampleKernel(std::vector<Vector3dF>& ssaoKernel) {
		for (unsigned int i = 0; i < 64; ++i)
		{
			Vector3dF sample(
				random0_1<float>() * 2.0 - 1.0, 
				random0_1<float>() * 2.0 - 1.0, 
				random0_1<float>());
			sample = sample.Normalize();
			sample *= random0_1<float>();
			float scale = float(i) / 64.0;

			// scale samples s.t. they're more aligned to center of kernel
			scale = lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			ssaoKernel.push_back(sample);
		}
	}
    
    static void generateNormals(SubMesh& mesh, bool reverse = false) {
        // printf("generateNormals %d\n", mesh.indexes.size() / 3);
        for (int tri_idx = 0, tri_num = mesh.indexes.size() / 3; tri_idx < tri_num; tri_idx += 1) {
            int vIdxes[3];
            vIdxes[0] = mesh.indexes[tri_idx * 3];
            vIdxes[1] = mesh.indexes[tri_idx * 3 + 1];
            vIdxes[2] = mesh.indexes[tri_idx * 3 + 2];
            // printf("%d,%d,%d\n", vIdxes[0], vIdxes[1], vIdxes[2]);
            Vertex& v0 = mesh.vertices[vIdxes[0]];
            Vertex& v1 = mesh.vertices[vIdxes[1]];
            Vertex& v2 = mesh.vertices[vIdxes[2]];
            const Vector3dF& p0 = v0.position;
            const Vector3dF& p1 = v1.position;
            const Vector3dF& p2 = v2.position;
            const Vector3dF e1 = p1 - p0;
            const Vector3dF e2 = p2 - p0;
            const Vector3dF faceNormal = (e1.Cross(e2));
            v0.normal += faceNormal;
            v1.normal += faceNormal;
            v2.normal += faceNormal;
        }
        for (int vIdx = 0; vIdx < mesh.vertices.size(); vIdx++) {
            Vertex& v = mesh.vertices[vIdx];
            v.normal = v.normal.Normalize();
            if (reverse) {
                v.normal = -v.normal;
            }
        }
    }

	static void generateQuadBoxMesh(Mesh& m, bool reverse = false) {
		m.meshes.emplace_back();
		SubMesh& mesh = *std::prev(m.meshes.end());
		mesh.vertices = Vertices{
			Vertex({ -0.5, -0.5, 0.5 }), // 左下
			Vertex({ 0.5, -0.5, 0.5 }),  // 右下
			Vertex({ 0.5,  0.5, 0.5 }),  // 右上
			Vertex({ -0.5,  0.5, 0.5 }), // 左上

			Vertex({ -0.5, -0.5,-0.5 }), // 左下-后
			Vertex({ 0.5, -0.5,-0.5 }),  // 右下-后
			Vertex({ 0.5,  0.5,-0.5 }),  // 右上-后
			Vertex({ -0.5,  0.5,-0.5 })  // 左上-后
		};
		mesh.indexes = {
			0, 1, 1, 2, 2, 3, 3, 0, // front
			1, 5, 5, 6, 6, 2, 2, 1, // right
			5, 4, 4, 7, 7, 6, 6, 5, // back
			4, 0, 0, 3, 3, 7, 7, 4, // left
			3, 2, 2, 6, 6, 7, 7, 3, // top
			4, 5, 5, 1, 1, 0, 0, 4, // bottom
		};
		generateNormals(mesh, reverse);
	}

    static void generateTriBoxMesh(Mesh& m, bool reverse = false) {
        m.meshes.emplace_back();
        SubMesh& mesh = *std::prev(m.meshes.end());
        mesh.vertices = Vertices{
            Vertex({-0.5, -0.5, 0.5}), // 左下
            Vertex({ 0.5, -0.5, 0.5}), // 右下
            Vertex({-0.5,  0.5, 0.5}), // 左上
            Vertex({ 0.5,  0.5, 0.5}), // 右上
            
            Vertex({-0.5,  0.5,-0.5}), // 左上-后
            Vertex({ 0.5,  0.5,-0.5}), // 右上-后
            Vertex({-0.5, -0.5,-0.5}), // 左下-后
            Vertex({ 0.5, -0.5,-0.5})  // 右下-后
        };
        mesh.indexes = {
            0, 1, 2,
            2, 1, 3,
            2, 3, 4,
            4, 3, 5,
            4, 5, 6,
            6, 5, 7,
            6, 7, 0,
            0, 7, 1,
            1, 7, 3,
            3, 7, 5,
            6, 0, 4,
            4, 0, 2
        };
        generateNormals(mesh, reverse);
    }

    static void generateSkyBoxMesh(Mesh& m) {
        m.meshes.emplace_back();
        SubMesh& mesh = *std::prev(m.meshes.end());
        mesh.vertices = Vertices{
			Vertex({ -1.0f,  1.0f, -1.0f }),
            Vertex({ -1.0f, -1.0f, -1.0f }),
            Vertex({ 1.0f, -1.0f, -1.0f }),
            Vertex({ 1.0f, -1.0f, -1.0f }),
            Vertex({ 1.0f,  1.0f, -1.0f }),
            Vertex({ -1.0f,  1.0f, -1.0f }),

			Vertex({ -1.0f, -1.0f,  1.0f }),
			Vertex({ -1.0f, -1.0f, -1.0f }),
			Vertex({ -1.0f,  1.0f, -1.0f }),
			Vertex({ -1.0f,  1.0f, -1.0f }),
			Vertex({ -1.0f,  1.0f,  1.0f }),
			Vertex({ -1.0f, -1.0f,  1.0f }),

			Vertex({ 1.0f, -1.0f, -1.0f }),
			Vertex({ 1.0f, -1.0f,  1.0f }),
			Vertex({ 1.0f,  1.0f,  1.0f }),
			Vertex({ 1.0f,  1.0f,  1.0f }),
			Vertex({ 1.0f,  1.0f, -1.0f }),
			Vertex({ 1.0f, -1.0f, -1.0f }),

			Vertex({ -1.0f, -1.0f,  1.0f }),
			Vertex({ -1.0f,  1.0f,  1.0f }),
			Vertex({ 1.0f,  1.0f,  1.0f }),
			Vertex({ 1.0f,  1.0f,  1.0f }),
			Vertex({ 1.0f, -1.0f,  1.0f }),
			Vertex({ -1.0f, -1.0f,  1.0f }),

			Vertex({ -1.0f,  1.0f, -1.0f }),
			Vertex({ 1.0f,  1.0f, -1.0f }),
			Vertex({ 1.0f,  1.0f,  1.0f }),
			Vertex({ 1.0f,  1.0f,  1.0f }),
			Vertex({ -1.0f,  1.0f,  1.0f }),
			Vertex({ -1.0f,  1.0f, -1.0f }),

			Vertex({ -1.0f, -1.0f, -1.0f }),
			Vertex({ -1.0f, -1.0f,  1.0f }),
			Vertex({ 1.0f, -1.0f, -1.0f }),
			Vertex({ 1.0f, -1.0f, -1.0f }),
			Vertex({ -1.0f, -1.0f,  1.0f }),
			Vertex({ 1.0f, -1.0f,  1.0f })
        };
    }
}
#endif
