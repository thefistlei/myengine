/******************************************************************************
 * File: Animator.h
 * Description: Animator drives animation playback and computes final bone transforms
 ******************************************************************************/

#pragma once

#include "Animation.h"
#include <vector>

namespace MyEngine {

class Animator {
public:
    Animator(Animation* animation);
    
    void UpdateAnimation(float dt);
    void PlayAnimation(Animation* pAnimation);
    void CalculateBoneTransform(const AssimpNodeData* node, const Mat4& parentTransform);
    
    std::vector<Mat4> GetFinalBoneMatrices() const { return m_FinalBoneMatrices; }

private:
    std::vector<Mat4> m_FinalBoneMatrices;
    Animation* m_CurrentAnimation;
    float m_CurrentTime;
    float m_DeltaTime;
};

} // namespace MyEngine
