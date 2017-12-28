#include "stdafx.h"
#include "system/animationSys.hpp"
#include "com/vertex.hpp"

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
        // Maps the dynamic buffer and update it.
        glBindBuffer(GL_ARRAY_BUFFER, dynamic_array_bo_);
        const size_t vbo_size = instance_count * 16 * sizeof(float);
        glBufferData(GL_ARRAY_BUFFER, vbo_size, uniforms, GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // Renders models.
        for (int i = 0; i < (draw_joints ? 2 : 1); ++i) {
            const Model& model = models_[i];
            
            // Setup model vertex data.
            glBindBuffer(GL_ARRAY_BUFFER, model.vbo);
            
            // Bind shader
            model.shader->Bind(transform, camera_->view_proj(), sizeof(Vertex), 0,
                               sizeof(Vertex), 12, sizeof(Vertex), 24);
            
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            
            // Setup instanced GL context.
            const GLint joint_attrib = model.shader->joint_instanced_attrib();
            glBindBuffer(GL_ARRAY_BUFFER, dynamic_array_bo_);
            glEnableVertexAttribArray(joint_attrib + 0);
            glEnableVertexAttribArray(joint_attrib + 1);
            glEnableVertexAttribArray(joint_attrib + 2);
            glEnableVertexAttribArray(joint_attrib + 3);
            glVertexAttribDivisor(joint_attrib + 0, 1);
            glVertexAttribDivisor(joint_attrib + 1, 1);
            glVertexAttribDivisor(joint_attrib + 2, 1);
            glVertexAttribDivisor(joint_attrib + 3, 1);
            glVertexAttribPointer(joint_attrib + 0, 4, GL_FLOAT, GL_FALSE,
                                   sizeof(ozz::math::Float4x4), GL_PTR_OFFSET(0));
            glVertexAttribPointer(joint_attrib + 1, 4, GL_FLOAT, GL_FALSE,
                                   sizeof(ozz::math::Float4x4), GL_PTR_OFFSET(16));
            glVertexAttribPointer(joint_attrib + 2, 4, GL_FLOAT, GL_FALSE,
                                  sizeof(ozz::math::Float4x4), GL_PTR_OFFSET(32));
            glVertexAttribPointer(joint_attrib + 3, 4, GL_FLOAT, GL_FALSE,
                                   sizeof(ozz::math::Float4x4), GL_PTR_OFFSET(48));
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            
            glDrawArraysInstanced(model.mode, 0, model.count, instance_count);
            
            glDisableVertexAttribArray(joint_attrib + 0);
            glDisableVertexAttribArray(joint_attrib + 1);
            glDisableVertexAttribArray(joint_attrib + 2);
            glDisableVertexAttribArray(joint_attrib + 3);
            glVertexAttribDivisor(joint_attrib + 0, 0);
            glVertexAttribDivisor(joint_attrib + 1, 0);
            glVertexAttribDivisor(joint_attrib + 2, 0);
            glVertexAttribDivisor(joint_attrib + 3, 0);
            
            model.shader->Unbind();
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
        const float kInter = .2f;
        {  // Prepares bone mesh.
            const Vector3dF pos[6] = {
                    {1.f, 0.f, 0.f},     {kInter, .1f, .1f},
                    {kInter, .1f, -.1f}, {kInter, -.1f, -.1f},
                    {kInter, -.1f, .1f}, {0.f, 0.f, 0.f}
                };
            const Vector3dF normals[8] = {
                (pos[2] - pos[1]).Cross(pos[2] - pos[0]).Normalize(),
                (pos[1] - pos[2]).Cross(pos[1] - pos[5]).Normalize(),
                (pos[3] - pos[2]).Cross(pos[3] - pos[0]).Normalize(),
                (pos[2] - pos[3]).Cross(pos[2] - pos[5]).Normalize(),
                (pos[4] - pos[3]).Cross(pos[4] - pos[0]).Normalize(),
                (pos[3] - pos[4]).Cross(pos[3] - pos[5]).Normalize(),
                (pos[1] - pos[4]).Cross(pos[1] - pos[0]).Normalize(),
                (pos[4] - pos[1]).Cross(pos[4] - pos[5]).Normalize()};
            const Vertex bones[24] = {
                {pos[0], normals[0]}, {pos[2], normals[0]},
                {pos[1], normals[0]}, {pos[5], normals[1]},
                {pos[1], normals[1]}, {pos[2], normals[1]},
                {pos[0], normals[2]}, {pos[3], normals[2]},
                {pos[2], normals[2]}, {pos[5], normals[3]},
                {pos[2], normals[3]}, {pos[3], normals[3]},
                {pos[0], normals[4]}, {pos[4], normals[4]},
                {pos[3], normals[4]}, {pos[5], normals[5]},
                {pos[3], normals[5]}, {pos[4], normals[5]},
                {pos[0], normals[6]}, {pos[1], normals[6]},
                {pos[4], normals[6]}, {pos[5], normals[7]},
                {pos[4], normals[7]}, {pos[1], normals[7]}};
            
            // Builds and fills the vbo.
            Model& bone = models_[0];
            bone.mode = GL_TRIANGLES;
            bone.count = OZZ_ARRAY_SIZE(bones);
            glGenBuffers(1, &bone.vbo);
            glBindBuffer(GL_ARRAY_BUFFER, bone.vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(bones), bones, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);  // Unbinds.
            
            // Init bone shader.
            bone.shader = BoneShader::Build();
            if (!bone.shader) {
                return false;
            }
        }
        
        {  // Prepares joint mesh.
            const int kNumSlices = 20;
            const int kNumPointsPerCircle = kNumSlices + 1;
            const int kNumPointsYZ = kNumPointsPerCircle;
            const int kNumPointsXY = kNumPointsPerCircle + kNumPointsPerCircle / 4;
            const int kNumPointsXZ = kNumPointsPerCircle;
            const int kNumPoints = kNumPointsXY + kNumPointsXZ + kNumPointsYZ;
            const float kRadius = kInter;  // Radius multiplier.
            Vertex joints[kNumPoints];
            
            // Fills vertices.
            int index = 0;
            for (int j = 0; j < kNumPointsYZ; ++j) {  // YZ plan.
                float angle = j * ozz::math::k2Pi / kNumSlices;
                float s = sinf(angle), c = cosf(angle);
                Vertex& vertex = joints[index++];
                vertex.position = Vector3dF(0.f, c * kRadius, s * kRadius);
                vertex.normal = Vector3dF(0.f, c, s);
            }
            for (int j = 0; j < kNumPointsXY; ++j) {  // XY plan.
                float angle = j * ozz::math::k2Pi / kNumSlices;
                float s = sinf(angle), c = cosf(angle);
                Vertex& vertex = joints[index++];
                vertex.position = Vector3dF(s * kRadius, c * kRadius, 0.f);
                vertex.normal = Vector3dF(s, c, 0.f);
            }
            for (int j = 0; j < kNumPointsXZ; ++j) {  // XZ plan.
                float angle = j * ozz::math::k2Pi / kNumSlices;
                float s = sinf(angle), c = cosf(angle);
                Vertex& vertex = joints[index++];
                vertex.position = Vector3dF(c * kRadius, 0.f, -s * kRadius);
                vertex.normal = Vector3dF(c, 0.f, -s);
            }
            assert(index == kNumPoints);
            
            // Builds and fills the vbo.
            Model& joint = models_[1];
            joint.mode = GL_LINE_STRIP;
            joint.count = OZZ_ARRAY_SIZE(joints);
            GL(GenBuffers(1, &joint.vbo));
            GL(BindBuffer(GL_ARRAY_BUFFER, joint.vbo));
            glBufferData(GL_ARRAY_BUFFER, sizeof(joints), joints, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);  // Unbinds.
            
            // Init joint shader.
            joint.shader = JointShader::Build();
            if (!joint.shader) {
                return false;
            }
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
