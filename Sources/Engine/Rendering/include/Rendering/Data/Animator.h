#pragma once
#include "Rendering/Resources/Animation.h"

#define MAX_BONE 200

namespace Rendering::Data
{
    class Animator
    {
    public:
        std::vector<Maths::FMatrix4> mFinalBoneMatrices;
        Rendering::Resources::Animation* mCurrentAnimation = nullptr;
        float mCurrentTime = 0;
        float duration = -1;

        Animator();
        void setAnimationClip(Rendering::Resources::Animation* pAnimation);
        void updateAnimation(float pDeltaTime, bool looping);
        void calculateBoneTransform(const Rendering::Resources::NodeData* pNode, Maths::FMatrix4 pParentTransform);
    
        void reset();
    };
}