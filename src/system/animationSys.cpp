#include "stdafx.h"
#include "system/animationSys.hpp"
#include "com/vertex.hpp"
#include "com/mesh.hpp"
#include "com/shader.hpp"
#include "com/cameraCom.hpp"
#include "utils/helper.hpp"
#include "event/bufferEvent.hpp"
#include "event/shaderEvent.hpp"
#include "utils/glutils.hpp"


using namespace std;


// Convenient macro definition for specifying buffer offsets.
#define GL_PTR_OFFSET(i) reinterpret_cast<void*>(static_cast<intptr_t>(i))

Animation a;
AnimationCom com;

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

ScratchBuffer scratch_buffer;


namespace renderer {
    void AnimationSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
        printf("AnimationSystem init\n");
        assert(InitPostureRendering());

        ozz::options::internal::Registrer<ozz::options::StringOption> OPTIONS_skeleton(
            "skeleton", "", "assets/animation/skeleton.ozz", false);
        // Reading skeleton.
        if (!LoadSkeleton(OPTIONS_skeleton, &a.skeleton)) {
            return;
        }
        ozz::options::internal::Registrer<ozz::options::StringOption> OPTIONS_animation(
            "animation", "", "assets/animation/animation.ozz", false);
        // Reading animation.
        if (!LoadAnimation(OPTIONS_animation, &a.animation)) {
            return;
        }
        ozz::memory::Allocator* allocator = ozz::memory::default_allocator();
        // Allocates runtime buffers.
        const int num_soa_joints = a.skeleton.num_soa_joints();
        com.locals = allocator->AllocateRange<ozz::math::SoaTransform>(num_soa_joints);
        const int num_joints = a.skeleton.num_joints();
        com.models = allocator->AllocateRange<ozz::math::Float4x4>(num_joints);
        // Allocates a cache that matches animation requirements.
        com.cache = allocator->New<ozz::animation::SamplingCache>(num_joints);
    }
    
    void AnimationSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {
        // Updates current animation time.
        UpdateAnimationTime(com, dt);
        
        // Samples optimized animation at t = animation_time_.
        ozz::animation::SamplingJob sampling_job;
        sampling_job.animation = &a.animation;
        sampling_job.cache = com.cache;
        sampling_job.time = com.time;
        sampling_job.output = com.locals;
        if (!sampling_job.Run()) {
            return;
        }
        
        // Converts from local space to model space matrices.
        ozz::animation::LocalToModelJob ltm_job;
        ltm_job.skeleton = &a.skeleton;
        ltm_job.input = com.locals;
        ltm_job.output = com.models;
        if (!ltm_job.Run()) {
            return;
        }
        DrawPosture(a.skeleton, com.models, ozz::math::Float4x4::identity());
    }
    
    bool AnimationSystem::DrawPosture(const ozz::animation::Skeleton& skeleton,
                     ozz::Range<const ozz::math::Float4x4> matrices,
                     const ozz::math::Float4x4& transform,
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
        
        DrawPosture_InstancedImpl(transform, uniforms, instance_count, draw_joints);

        return true;
    }
    
    
    // "Draw posture" internal instanced rendering implementation.
    void AnimationSystem::DrawPosture_InstancedImpl(
                                                 const ozz::math::Float4x4& transform, const float* uniforms,
                                                 int instance_count, bool draw_joints) {
        m_evtMgr->emit<CreateInstanceBufferEvent>("posture");
        m_evtMgr->emit<UpdateInstanceBufferEvent>("posture",
                                                  instance_count,
                                                  16 * sizeof(float),
                                                  (void*)uniforms);
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
            m_evtMgr->emit<UploadCameraToShaderEvent>(objCamera, shader);

            // Bind shader
           // model.shader->Bind(transform, camera_->view_proj(), sizeof(Vertex), 0,
            //                   sizeof(Vertex), 12, sizeof(Vertex), 24);
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
            uniform += 16;
            
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
        // m_evtMgr->emit<CreateMeshBufferEvent>(meshID);
        
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
            subMesh.vertices.reserve(kNumPoints);
            subMesh.meshType = MeshType::LineStrip;
            // Fills vertices.
            int index = 0;
            for (int j = 0; j < kNumPointsYZ; ++j) {  // YZ plan.
                float angle = j * ozz::math::k2Pi / kNumSlices;
                float s = sinf(angle), c = cosf(angle);
                Vertex& vertex = subMesh.vertices[index++];
                vertex.position = Vector3dF(0.f, c * kRadius, s * kRadius);
                vertex.normal = Vector3dF(0.f, c, s);
            }
            for (int j = 0; j < kNumPointsXY; ++j) {  // XY plan.
                float angle = j * ozz::math::k2Pi / kNumSlices;
                float s = sinf(angle), c = cosf(angle);
                Vertex& vertex = subMesh.vertices[index++];
                vertex.position = Vector3dF(s * kRadius, c * kRadius, 0.f);
                vertex.normal = Vector3dF(s, c, 0.f);
            }
            for (int j = 0; j < kNumPointsXZ; ++j) {  // XZ plan.
                float angle = j * ozz::math::k2Pi / kNumSlices;
                float s = sinf(angle), c = cosf(angle);
                Vertex& vertex = subMesh.vertices[index++];
                vertex.position = Vector3dF(c * kRadius, 0.f, -s * kRadius);
                vertex.normal = Vector3dF(c, 0.f, -s);
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
    
    void AnimationSystem::UpdateAnimationTime(AnimationCom com, float dt) {
        const float new_time = com.time + dt * com.playback_speed;
        const float loops = new_time / a.animation.duration();
        com.time = new_time - floorf(loops) * a.animation.duration();
    }
    
};
