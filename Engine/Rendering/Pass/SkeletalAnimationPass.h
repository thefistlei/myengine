/******************************************************************************
 * File: SkeletalAnimationPass.h
 * Description: Skeletal animation rendering pass (from LearnOpenGL example)
 ******************************************************************************/

#pragma once

#include "RenderPass.h"
#include "Animation/AnimatedModelLoader.h"
#include "Animation/Animator.h"
#include "Animation/AnimatedMesh.h"
#include "Animation/Animation.h"
#include <cstdint>
#include <memory>

using GLuint = unsigned int;  // Forward declare GLuint type

namespace MyEngine {

class SkeletalAnimationPass : public RenderPass {
public:
    SkeletalAnimationPass() = default;
    virtual ~SkeletalAnimationPass() = default;
    
    // Meta information
    const char* GetName() const override { return "SkeletalAnimationPass"; }
    PassType GetType() const override { return PassType::Graphics; }
    PassCategory GetCategory() const override { return PassCategory::VFX; }

    void OnCreate(RenderBackend* backend) override;
    void Execute(const SceneView& view, Registry* registry) override;
    void OnDestroy() override;
    void OnGUI() override;  // Editor UI for properties
    
    // Getters/Setters for properties
    void SetPosition(const Vec3& pos) { m_Position = pos; }
    Vec3 GetPosition() const { return m_Position; }
    
    void SetScale(const Vec3& scale) { m_Scale = scale; }
    Vec3 GetScale() const { return m_Scale; }

private:
    void CreateAnimatedModel();
    void CreateAnimationShader();
    void LoadTextures();

    RenderBackend* m_Backend = nullptr;
    
    // Animation resources
    std::unique_ptr<AnimatedModelLoader> m_ModelLoader;
    std::shared_ptr<AnimatedMesh> m_AnimatedMesh;
    std::shared_ptr<Animation> m_Animation;
    std::unique_ptr<Animator> m_Animator;
    
    // Rendering resources
    std::shared_ptr<Shader> m_Shader;
    GLuint m_DiffuseTextureID = 0;  // Loaded texture
    GLuint m_WhiteTexture = 0;       // Fallback
    
    // Model transform
    Vec3 m_Position = Vec3(0, -0.4f, 0);
    Vec3 m_Scale = Vec3(0.5f, 0.5f, 0.5f);
};

} // namespace MyEngine
