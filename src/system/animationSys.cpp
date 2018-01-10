#include "stdafx.h"
#include "system/animationSys.hpp"
#include "com/vertex.hpp"
#include "com/mesh.hpp"
#include "com/spatialData.hpp"
#include "com/shader.hpp"
#include "com/cameraCom.hpp"
#include "utils/helper.hpp"
#include "event/bufferEvent.hpp"
#include "event/shaderEvent.hpp"
#include "event/materialEvent.hpp"
#include "event/miscEvent.hpp"
#include "utils/glutils.hpp"


using namespace std;

const uint8_t kDefaultColorsArray[][4] = {
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255},
    {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}};


// Convenient macro definition for specifying buffer offsets.
#define GL_PTR_OFFSET(i) reinterpret_cast<void*>(static_cast<intptr_t>(i))

class ScratchBuffer {
public:
    ScratchBuffer() : buffer(NULL), size(0) {}
    ~ScratchBuffer() {
        ozz::memory::default_allocator()->Deallocate(buffer);
    }
    void* Resize(size_t s) {
        if (s > size) {
            size = s;
            buffer = ozz::memory::default_allocator()->Reallocate(buffer, s, 16);
        }
        return buffer;
    }
    
private:
    void* buffer;
    size_t size;
};

static ScratchBuffer scratch_buffer;


namespace renderer {
    void AnimationSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
        printf("AnimationSystem init\n");
        evtMgr.on<DebugDrawSkeletonEvent>(*this);
        evtMgr.on<LoadAnimationEvent>(*this);
        evtMgr.on<GlobalComAddedEvent>(*this);
    }
    
    void AnimationSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {
        auto dataSet = m_objMgr->getSingletonComponent<AnimationDataSet>();
        for (const Object obj : m_objMgr->entities<AnimationCom>()) {
            auto com = obj.component<AnimationCom>();
            AnimationData& data = dataSet->getAnimationData(com->aniDataID);
            UpdateAnimationTime(data, com, dt);
            ozz::animation::Animation* ani = data.GetAnimation(com->curAniName);
            DoSamplingJob(com->time, com->cache, ani, com->locals);
            DoLocalToModelJob(data.skeleton, com->locals, com->models);
        }
    }
    
    void AnimationSystem::receive(const DebugDrawSkeletonEvent &evt) {
        auto dataSet = m_objMgr->getSingletonComponent<AnimationDataSet>();
        for (const Object obj : m_objMgr->entities<AnimationCom>()) {
            auto com = obj.component<AnimationCom>();
            AnimationData& data = dataSet->getAnimationData(com->aniDataID);
            // DrawPosture(data.skeleton, com->models, obj);
           
            assert(com->models.Count() == com->skinning_matrices.Count() &&
                   com->models.Count() == data.mesh.inverse_bind_poses.size());
            for (size_t i = 0; i < com->models.Count(); ++i) {
                com->skinning_matrices[i] = com->models[i] * data.mesh.inverse_bind_poses[i];
            }
            DrawSkinnedMesh(data.mesh, com->skinning_matrices, obj);
        }
    }
    
    void AnimationSystem::receive(const LoadAnimationEvent &evt) {
        LoadByConfig(evt.assetsDir, evt.config);
    }
    
    void AnimationSystem::receive(const GlobalComAddedEvent &evt) {
        assert(InitPostureRendering());
    }
    
    void AnimationSystem::DoSamplingJob(float time,
                       ozz::animation::SamplingCache* cache,
                       ozz::animation::Animation* animation,
                       ozz::Range<ozz::math::SoaTransform> locals) {
        ozz::animation::SamplingJob sampling_job;
        sampling_job.animation = animation;
        sampling_job.cache = cache;
        sampling_job.time = time;
        sampling_job.output = locals;
        assert(sampling_job.Run());
    }
    
    void AnimationSystem::DoLocalToModelJob(ozz::animation::Skeleton& skeleton,
                                            ozz::Range<ozz::math::SoaTransform> locals,
                                            ozz::Range<ozz::math::Float4x4> models) {
        // Converts from local space to model space matrices.
        ozz::animation::LocalToModelJob ltm_job;
        ltm_job.skeleton = &skeleton;
        ltm_job.input = locals;
        ltm_job.output = models;
        assert(ltm_job.Run());
    }
    
    
    bool AnimationSystem::LoadByConfig(const std::string& assetsDir, const json &config) {
        for (auto aniDataInfo : config)
        {
            std::string aniDataName = aniDataInfo["name"];
            std::string meshFileName = aniDataInfo["mesh"];
            std::string skeletonFileName = aniDataInfo["skeleton"];
            LoadMeshAndSkeleton(assetsDir, aniDataName, meshFileName, skeletonFileName);
            if (!aniDataInfo["aniDict"].is_null()) {
                for (auto info : aniDataInfo["aniDict"])
                {
                    std::string aniAliasName = info["name"];
                    std::string aniFileName = info["file"];
                    LoadAnimation(assetsDir, aniDataName, aniAliasName, aniFileName);
                }
            }
            if (!aniDataInfo["material"].is_null()) {
                auto com = m_objMgr->getSingletonComponent<AnimationDataSet>();
                AnimationData& aniData = com->getAnimationData(aniDataName);
                auto matSet = m_objMgr->getSingletonComponent<MaterialSet>();
                aniData.mesh.settingIDs.push_back(matSet->alias2id[aniDataInfo["material"]]);
                
            }
        }
        return true;
    }
    
    bool AnimationSystem::LoadMeshAndSkeleton(const std::string& assetsDir, std::string& aniDataName, std::string& meshFileName, std::string& skeletonFileName) {
        auto com = m_objMgr->getSingletonComponent<AnimationDataSet>();
        if (!com->hasAnimationData(aniDataName)) {
            auto id = com->newAnimationDataID();
            com->animations.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple());
            com->alias2id.emplace(std::piecewise_construct, std::forward_as_tuple(aniDataName), std::forward_as_tuple(id));
        }
        AnimationData& data = com->getAnimationData(aniDataName);
        if (!LoadMesh((assetsDir + meshFileName).c_str(), &data.mesh)) {
            printf("LoadMesh failed %s %s", aniDataName.c_str(), meshFileName.c_str());
            return false;
        }
        if (skeletonFileName != "") {
            ozz::options::internal::Registrer<ozz::options::StringOption> OPTIONS_skeleton(
                "skeleton", "", (assetsDir + skeletonFileName).c_str(), false);
            if (!LoadSkeleton(OPTIONS_skeleton, &data.skeleton)) {
                printf("LoadSkeleton failed %s %s", aniDataName.c_str(), skeletonFileName.c_str());
                return false;
            }
        }
        return true;
    }
    
    bool AnimationSystem::LoadAnimation(const std::string& assetsDir, std::string& aniDataName, std::string& aniAliasName, std::string& aniFileName) {
        auto com = m_objMgr->getSingletonComponent<AnimationDataSet>();
        if (!com->hasAnimationData(aniDataName)) {
            printf("please LoadSkeleton before LoadAnimation %s", aniDataName.c_str());
            return false;
        }
        AnimationData& data = com->getAnimationData(aniDataName);
        ozz::options::internal::Registrer<ozz::options::StringOption> OPTIONS_animation(
            "animation", "", (assetsDir + aniFileName).c_str(), false);
        data.aniDict.emplace(std::piecewise_construct, std::forward_as_tuple(aniAliasName),
			std::forward_as_tuple());
        ozz::animation::Animation& animation = data.aniDict[aniAliasName];
        // Reading animation.
        if (!LoadAnimation(OPTIONS_animation, &animation)) {
            printf("LoadAnimation failed %s %s", aniDataName.c_str(), aniFileName.c_str());
            return false;
        }
        return true;
    }
    
    bool AnimationSystem::DoSkinningJob(const OzzMesh& mesh,
             const ozz::Range<ozz::math::Float4x4> skinning_matrices,
             void* &vbo_map,
             GLsizei &vbo_size) {
        const int vertex_count = mesh.vertex_count();
        size_t processed_vertex_count = 0;
        // Positions and normals are interleaved to improve caching while executing
        // skinning job.
        const GLsizei positions_offset = 0;
        const GLsizei normals_offset = sizeof(float) * 3;
        const GLsizei uvs_offset = sizeof(float) * 6;
        const GLsizei tangents_offset = sizeof(float) * 8;
        const GLsizei positions_stride = sizeof(float) * 11;
        const GLsizei normals_stride = positions_stride;
        const GLsizei tangents_stride = positions_stride;
        const GLsizei uvs_stride = positions_stride;
        const GLsizei skinned_data_size = vertex_count * positions_stride;
        vbo_size = skinned_data_size;
        vbo_map = scratch_buffer.Resize(vbo_size);
        
        // Iterate mesh parts and fills vbo.
        // Runs a skinning job per mesh part. Triangle indices are shared
        // across parts.
        for (size_t i = 0; i < mesh.parts.size(); ++i) {
            const OzzMesh::Part& part = mesh.parts[i];
            
            // Skip this iteration if no vertex.
            const size_t part_vertex_count = part.positions.size() / 3;
            if (part_vertex_count == 0) {
                continue;
            }
            
            // Fills the job.
            ozz::geometry::SkinningJob skinning_job;
            skinning_job.vertex_count = static_cast<int>(part_vertex_count);
            const int part_influences_count = part.influences_count();
            
            // Clamps joints influence count according to the option.
            skinning_job.influences_count = part_influences_count;
            
            // Setup skinning matrices, that came from the animation stage before being
            // multiplied by inverse model-space bind-pose.
            skinning_job.joint_matrices = skinning_matrices;
            
            // Setup joint's indices.
            skinning_job.joint_indices = make_range(part.joint_indices);
            skinning_job.joint_indices_stride =
            sizeof(uint16_t) * part_influences_count;
            
            // Setup joint's weights.
            if (part_influences_count > 1) {
                skinning_job.joint_weights = make_range(part.joint_weights);
                skinning_job.joint_weights_stride =
                sizeof(float) * (part_influences_count - 1);
            }
            
            // Setup input positions, coming from the loaded mesh.
            skinning_job.in_positions = make_range(part.positions);
            skinning_job.in_positions_stride =
            sizeof(float) * OzzMesh::Part::kPositionsCpnts;
            
            // Setup output positions, coming from the rendering output mesh buffers.
            // We need to offset the buffer every loop.
            skinning_job.out_positions.begin = reinterpret_cast<float*>(
                ozz::PointerStride(vbo_map, positions_offset + processed_vertex_count * positions_stride));
            skinning_job.out_positions.end = ozz::PointerStride(
                skinning_job.out_positions.begin, part_vertex_count * positions_stride);
            skinning_job.out_positions_stride = positions_stride;
            
            // Setup normals if input are provided.
            float* out_normal_begin = reinterpret_cast<float*>(ozz::PointerStride(
                vbo_map, normals_offset + processed_vertex_count * normals_stride));
            const float* out_normal_end = ozz::PointerStride(
                out_normal_begin, part_vertex_count * normals_stride);
            
            if (part.normals.size() / OzzMesh::Part::kNormalsCpnts ==
                part_vertex_count) {
                // Setup input normals, coming from the loaded mesh.
                skinning_job.in_normals = make_range(part.normals);
                skinning_job.in_normals_stride =
                sizeof(float) * OzzMesh::Part::kNormalsCpnts;
                
                // Setup output normals, coming from the rendering output mesh buffers.
                // We need to offset the buffer every loop.
                skinning_job.out_normals.begin = out_normal_begin;
                skinning_job.out_normals.end = out_normal_end;
                skinning_job.out_normals_stride = normals_stride;
            } else {
                // Fills output with default normals.
                for (float* normal = out_normal_begin; normal < out_normal_end;
                     normal = ozz::PointerStride(normal, normals_stride)) {
                    normal[0] = 0.f;
                    normal[1] = 1.f;
                    normal[2] = 0.f;
                }
            }
            
            // Setup tangents if input are provided.
            float* out_tangent_begin = reinterpret_cast<float*>(ozz::PointerStride(
                vbo_map, tangents_offset + processed_vertex_count * tangents_stride));
            const float* out_tangent_end = ozz::PointerStride(
                out_tangent_begin, part_vertex_count * tangents_stride);
            
            if (part.tangents.size() / OzzMesh::Part::kTangentsCpnts ==
                part_vertex_count) {
                // Setup input tangents, coming from the loaded mesh.
                skinning_job.in_tangents = make_range(part.tangents);
                skinning_job.in_tangents_stride =
                sizeof(float) * OzzMesh::Part::kTangentsCpnts;
                
                // Setup output tangents, coming from the rendering output mesh buffers.
                // We need to offset the buffer every loop.
                skinning_job.out_tangents.begin = out_tangent_begin;
                skinning_job.out_tangents.end = out_tangent_end;
                skinning_job.out_tangents_stride = tangents_stride;
            } else {
                // Fills output with default tangents.
                for (float* tangent = out_tangent_begin; tangent < out_tangent_end;
                     tangent = ozz::PointerStride(tangent, tangents_stride)) {
                    tangent[0] = 1.f;
                    tangent[1] = 0.f;
                    tangent[2] = 0.f;
                }
            }
            
            // Execute the job, which should succeed unless a parameter is invalid.
            if (!skinning_job.Run()) {
                return false;
            }
        
            const size_t part_uvs_count =
            part.uvs.size() / OzzMesh::Part::kUVsCpnts;
            if (part_vertex_count == part_uvs_count) {
                float* out_uv_begin = reinterpret_cast<float*>(ozz::PointerStride(
                    vbo_map, uvs_offset + processed_vertex_count * uvs_stride));
                const float* out_uv_end = ozz::PointerStride(
                    out_uv_begin, part_vertex_count * uvs_stride);
                uint32_t i = 0;
                float* uv = out_uv_begin;
                for (; uv < out_uv_end; uv = ozz::PointerStride(uv, uvs_stride), i++) {
                    uv[0] = part.uvs[i + 0];
                    uv[1] = part.uvs[i + 1];
                }
            }
            // Some more vertices were processed.
            processed_vertex_count += part_vertex_count;
        }
        return true;
    }
    
    bool AnimationSystem::DrawSkinnedMesh(const OzzMesh& mesh, const ozz::Range<ozz::math::Float4x4> skinning_matrices, Object obj) {
        auto spatialData = obj.component<SpatialData>();
        const Matrix4x4& modelMat = spatialData->o2w.GetMatrix();
        auto spSetCom = m_objMgr->getSingletonComponent<ShaderProgramSet>();
        Shader shader = spSetCom->getShader("testAnimation");
        shader.use();
        // Forward to DrawMesh function is skinning is disabled.
        // if (skip_skinning) {
        //    return DrawMesh(mesh, obj);
        // }
        
        void* vbo_map = nullptr;
        GLsizei vbo_size = 0;
        if (!DoSkinningJob(mesh, skinning_matrices, vbo_map, vbo_size)) {
            return false;
        }
        
        auto settingID = mesh.settingIDs[0];
        m_evtMgr->emit<ActiveMaterialEvent>(settingID, shader);
        
        Object objCamera = m_objMgr->getSingletonComponent<PerspectiveCameraView>().object();
        m_evtMgr->emit<UploadCameraToShaderEvent>(objCamera, shader);
        shader.setMatrix4f("modelMat", modelMat);

        const OzzMesh::TriangleIndices& indices = mesh.triangle_indices;
        
        m_evtMgr->emit<CreateDynamicMeshBufferEvent>("skinned");
        m_evtMgr->emit<BindDynamicMeshBufferEvent>("skinned");
        m_evtMgr->emit<UpdateDynamicMeshBufferEvent>("skinned", vbo_size, vbo_map, indices.size() * sizeof(OzzMesh::TriangleIndices::value_type), array_begin(indices));
        
        // Draws the mesh.
        OZZ_STATIC_ASSERT(sizeof(OzzMesh::TriangleIndices::value_type) == 2);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()),
                       GL_UNSIGNED_SHORT, 0);
        
        m_evtMgr->emit<UnbindDynamicMeshBufferEvent>("skinned");
    
        CheckGLError;
        return true;
    }
    
    bool AnimationSystem::DrawPosture(const ozz::animation::Skeleton& skeleton,
                     ozz::Range<const ozz::math::Float4x4> matrices,
                     Object obj,
                     bool draw_joints) {
        if (!matrices.begin || !matrices.end) {
            return false;
        }
        if (matrices.end - matrices.begin < skeleton.num_joints()) {
            return false;
        }
        
        // Convert matrices to uniforms.
        const int max_skeleton_pieces = ozz::animation::Skeleton::kMaxJoints * 2;
        const size_t max_uniforms_size = max_skeleton_pieces * 2 * 16 * sizeof(float);
        float* uniforms =
        static_cast<float*>(scratch_buffer.Resize(max_uniforms_size));
        
        const int instance_count = DrawPosture_FillUniforms(skeleton, matrices, uniforms, max_skeleton_pieces);
        assert(instance_count <= max_skeleton_pieces);
        
        DrawPosture_InstancedImpl(obj, uniforms, instance_count, draw_joints);

        return true;
    }
    
    
    // "Draw posture" internal instanced rendering implementation.
    void AnimationSystem::DrawPosture_InstancedImpl(
                                                 Object obj, const float* uniforms,
                                                 int instance_count, bool draw_joints) {
        m_evtMgr->emit<CreateInstanceBufferEvent>("posture");
        m_evtMgr->emit<UpdateInstanceBufferEvent>("posture",
                                                  instance_count,
                                                  4 * 4 * sizeof(float),
                                                  (void*)uniforms, false);
        
        auto spatialData = obj.component<SpatialData>();
        const Matrix4x4& modelMat = spatialData->o2w.GetMatrix();
        
        auto meshSetCom = m_objMgr->getSingletonComponent<MeshSet>();
        auto spSetCom = m_objMgr->getSingletonComponent<ShaderProgramSet>();
        MeshID meshIDBone = meshSetCom->getMeshID("bone");
        MeshID meshIDJoint = meshSetCom->getMeshID("joint");
        
        Shader shaderBone = spSetCom->getShader("bone");
        Shader shaderJoint = spSetCom->getShader("joint");
        Object objCamera = m_objMgr->getSingletonComponent<PerspectiveCameraView>().object();

        // Renders models.
        for (int i = 0; i < (draw_joints ? 2 : 1); ++i) {
            MeshID meshID = i == 0? meshIDBone : meshIDJoint;
            Shader shader = i == 0? shaderBone : shaderJoint;
            shader.use();
            shader.setMatrix4f("modelMat", modelMat);
            m_evtMgr->emit<UploadCameraToShaderEvent>(objCamera, shader);
            m_evtMgr->emit<BindInstanceBufferEvent>(meshID, 0, "posture");
            m_evtMgr->emit<DrawMeshBufferEvent>(meshID, 0);
            m_evtMgr->emit<UnbindInstanceBufferEvent>(meshID, 0);
        }
    }
    
    
    int AnimationSystem::DrawPosture_FillUniforms(const ozz::animation::Skeleton& skeleton,
                                 ozz::Range<const ozz::math::Float4x4> matrices,
                                 float* uniforms, int max_instances) {
        assert(ozz::math::IsAligned(uniforms, OZZ_ALIGN_OF(ozz::math::SimdFloat4)));
        
        // Prepares computation constants.
        const int num_joints = skeleton.num_joints();
        const ozz::animation::Skeleton::JointProperties* properties =
        skeleton.joint_properties().begin;
        
        int instances = 0;
        for (int i = 0; i < num_joints && instances < max_instances; ++i) {
            // Root isn't rendered.
            const int parent_id = properties[i].parent;
            if (parent_id == ozz::animation::Skeleton::kNoParentIndex) {
                continue;
            }
            
            // Selects joint matrices.
            const ozz::math::Float4x4& parent = matrices.begin[parent_id];
            const ozz::math::Float4x4& current = matrices.begin[i];
            
            // Copy parent joint's raw matrix, to render a bone between the parent
            // and current matrix.
            float* uniform = uniforms + instances * 16;
            ozz::math::StorePtr(parent.cols[0], uniform + 0);
            ozz::math::StorePtr(parent.cols[1], uniform + 4);
            ozz::math::StorePtr(parent.cols[2], uniform + 8);
            ozz::math::StorePtr(parent.cols[3], uniform + 12);
            
            // Set bone direction (bone_dir). The shader expects to find it at index
            // [3,7,11] of the matrix.
            // Index 15 is used to store whether a bone should be rendered,
            // otherwise it's a leaf.
            float bone_dir[4];
            ozz::math::StorePtrU(current.cols[3] - parent.cols[3], bone_dir);
            uniform[3] = bone_dir[0];
            uniform[7] = bone_dir[1];
            uniform[11] = bone_dir[2];
            uniform[15] = 1.f;  // Enables bone rendering.
            
            // Next instance.
            ++instances;
            
            // Only the joint is rendered for leaves, the bone model isn't.
            if (properties[i].is_leaf) {
                // Copy current joint's raw matrix.
                uniform = uniforms + instances * 16;
                ozz::math::StorePtr(current.cols[0], uniform + 0);
                ozz::math::StorePtr(current.cols[1], uniform + 4);
                ozz::math::StorePtr(current.cols[2], uniform + 8);
                ozz::math::StorePtr(current.cols[3], uniform + 12);
                
                // Re-use bone_dir to fix the size of the leaf (same as previous bone).
                // The shader expects to find it at index [3,7,11] of the matrix.
                uniform[3] = bone_dir[0];
                uniform[7] = bone_dir[1];
                uniform[11] = bone_dir[2];
                uniform[15] = 0.f;  // Disables bone rendering.
                ++instances;
            }
        }
        
        return instances;
    }
    
    
    bool AnimationSystem::InitPostureRendering() {
        auto meshSetCom = m_objMgr->getSingletonComponent<MeshSet>();
        const float kInter = .2f;
        {
            // Prepares bone mesh.
            MeshID meshID;
            std::string name = "bone";
            Mesh& mesh = meshSetCom->newMesh(name, meshID);
            mesh.meshes.emplace_back();
            SubMesh& subMesh = mesh.meshes[0];
            const Vector3dF pos[6] = {
                    {1.f, 0.f, 0.f},     {kInter, .1f, .1f},
                    {kInter, .1f, -.1f}, {kInter, -.1f, -.1f},
                    {kInter, -.1f, .1f}, {0.f, 0.f, 0.f}
                };
            subMesh.vertices = {
                {{1.f, 0.f, 0.f}},     {{kInter, .1f, .1f}},
                {{kInter, .1f, -.1f}}, {{kInter, -.1f, -.1f}},
                {{kInter, -.1f, .1f}},  {{0.f, 0.f, 0.f}}
            };
            subMesh.indexes = {
                0, 2, 1,
                5, 1, 2,
                0, 3, 2,
                5, 2, 3,
                0, 4, 3,
                5, 3, 4,
                0, 1, 4,
                5, 4, 1
            };
            generateNormals(subMesh);
            m_evtMgr->emit<CreateMeshBufferEvent>(meshID);
        }
        
        {
            // Prepares joint mesh.
            MeshID meshID;
            std::string name = "joint";
            Mesh& mesh = meshSetCom->newMesh(name, meshID);
            mesh.meshes.emplace_back();
            SubMesh& subMesh = mesh.meshes[0];
            const int kNumSlices = 20;
            const int kNumPointsPerCircle = kNumSlices + 1;
            const int kNumPointsYZ = kNumPointsPerCircle;
            const int kNumPointsXY = kNumPointsPerCircle + kNumPointsPerCircle / 4;
            const int kNumPointsXZ = kNumPointsPerCircle;
            const int kNumPoints = kNumPointsXY + kNumPointsXZ + kNumPointsYZ;
            const float kRadius = kInter;  // Radius multiplier.
            subMesh.vertices.resize(kNumPoints);
            subMesh.meshType = MeshType::LineStrip;
            // Fills vertices.
            int index = 0;
            for (int j = 0; j < kNumPointsYZ; ++j) {  // YZ plan.
                float angle = j * ozz::math::k2Pi / kNumSlices;
                float s = sinf(angle), c = cosf(angle);
                Vertex& vertex = subMesh.vertices[index++];
                vertex.position = Vector3dF(0.f, c * kRadius, s * kRadius);
                vertex.normal = Vector3dF(0.f, c, s).Normalize();
            }
            for (int j = 0; j < kNumPointsXY; ++j) {  // XY plan.
                float angle = j * ozz::math::k2Pi / kNumSlices;
                float s = sinf(angle), c = cosf(angle);
                Vertex& vertex = subMesh.vertices[index++];
                vertex.position = Vector3dF(s * kRadius, c * kRadius, 0.f);
                vertex.normal = Vector3dF(s, c, 0.f).Normalize();
            }
            for (int j = 0; j < kNumPointsXZ; ++j) {  // XZ plan.
                float angle = j * ozz::math::k2Pi / kNumSlices;
                float s = sinf(angle), c = cosf(angle);
                Vertex& vertex = subMesh.vertices[index++];
                vertex.position = Vector3dF(c * kRadius, 0.f, -s * kRadius);
                vertex.normal = Vector3dF(c, 0.f, -s).Normalize();
            }
            assert(index == kNumPoints);
            m_evtMgr->emit<CreateMeshBufferEvent>(meshID);

        }
        
        return true;
    }
    
    bool AnimationSystem::LoadSkeleton(const char* _filename, ozz::animation::Skeleton* _skeleton) {
        assert(_filename && _skeleton);
        ozz::log::Out() << "Loading skeleton archive " << _filename << "."
        << std::endl;
        ozz::io::File file(_filename, "rb");
        if (!file.opened()) {
            ozz::log::Err() << "Failed to open skeleton file " << _filename << "."
            << std::endl;
            return false;
        }
        ozz::io::IArchive archive(&file);
        if (!archive.TestTag<ozz::animation::Skeleton>()) {
            ozz::log::Err() << "Failed to load skeleton instance from file "
            << _filename << "." << std::endl;
            return false;
        }
        
        // Once the tag is validated, reading cannot fail.
        archive >> *_skeleton;
        
        return true;
    }
    
    bool AnimationSystem::LoadAnimation(const char* _filename,
                       ozz::animation::Animation* _animation) {
        assert(_filename && _animation);
        ozz::log::Out() << "Loading animation archive: " << _filename << "."
        << std::endl;
        ozz::io::File file(_filename, "rb");
        if (!file.opened()) {
            ozz::log::Err() << "Failed to open animation file " << _filename << "."
            << std::endl;
            return false;
        }
        ozz::io::IArchive archive(&file);
        if (!archive.TestTag<ozz::animation::Animation>()) {
            ozz::log::Err() << "Failed to load animation instance from file "
            << _filename << "." << std::endl;
            return false;
        }
        
        // Once the tag is validated, reading cannot fail.
        archive >> *_animation;
        
        return true;
    }
    
    bool AnimationSystem::LoadMesh(const char* _filename, OzzMesh* ozzMesh) {
        ozz::sample::Mesh* mesh = ozzMesh;
        assert(_filename && mesh);
        ozz::log::Out() << "Loading mesh archive: " << _filename << "." << std::endl;
        ozz::io::File file(_filename, "rb");
        if (!file.opened()) {
            ozz::log::Err() << "Failed to open mesh file " << _filename << "."
            << std::endl;
            return false;
        }
        ozz::io::IArchive archive(&file);
        if (!archive.TestTag<ozz::sample::Mesh>()) {
            ozz::log::Err() << "Failed to load mesh instance from file " << _filename
            << "." << std::endl;
            return false;
        }
        
        // Once the tag is validated, reading cannot fail.
        archive >> *mesh;
        
        return true;
    }
    
    void AnimationSystem::UpdateAnimationTime(AnimationData& data, ComponentHandle<AnimationCom> com, float dt) {
        if (!com->play) {
            return;
        }
        ozz::animation::Animation* animation = data.GetAnimation(com->curAniName);
        const float new_time = com->time + dt * com->playback_speed;
        const float loops = new_time / animation->duration();
        com->time = new_time - floorf(loops) * animation->duration();
    }
    
};
