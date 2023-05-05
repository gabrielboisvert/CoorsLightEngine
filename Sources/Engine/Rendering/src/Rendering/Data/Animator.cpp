#include "Rendering/Data/Animator.h"
#include "Maths/Utils.h"

using namespace Rendering::Data;

Animator::Animator()
{
    mCurrentTime = 0.0;

    mFinalBoneMatrices.reserve(MAX_BONE);
    for (unsigned int i = 0; i < MAX_BONE; i++)
        mFinalBoneMatrices.push_back(Maths::FMatrix4(1.0f));
}

void Animator::reset()
{
    mCurrentTime = 0.0;
}

void Animator::setAnimationClip(Rendering::Resources::Animation* pAnimation)
{
    mCurrentAnimation = pAnimation;
    mCurrentTime = 0.0;
    duration = pAnimation == nullptr? -1 : pAnimation->mDuration;
}

void Animator::updateAnimation(float pDeltaTime, bool looping)
{
    if (mCurrentAnimation == nullptr)
        return;

    if (!looping)
        if (mCurrentTime >= duration)
            return;

    mCurrentTime += mCurrentAnimation->mTicksPerSecond * pDeltaTime;
    if (!looping)
        if (mCurrentTime >= duration)
        {
            mCurrentTime = duration;
            return;
        }

    mCurrentTime = std::fmod(mCurrentTime, mCurrentAnimation->mDuration);
    calculateBoneTransform(&mCurrentAnimation->mRootNode, Maths::FMatrix4(1.0f));
}

void Animator::calculateBoneTransform(const Rendering::Resources::NodeData* pNode, Maths::FMatrix4 pParentTransform)
{
    std::string nodeName = pNode->mName;
    Maths::FMatrix4 nodeTransform = pNode->mTransformation;

    Bone* Bone = mCurrentAnimation->findBone(nodeName);

    if (Bone)
    {
        Bone->update(mCurrentTime);
        nodeTransform = Bone->mLocalTransform;
    }

    Maths::FMatrix4 globalTransformation = pParentTransform * nodeTransform;

    std::map<std::string, BoneInfo>& boneInfoMap = mCurrentAnimation->mBoneInfoMap;
    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        int index = boneInfoMap[nodeName].mId;
        Maths::FMatrix4 offset = boneInfoMap[nodeName].mOffset;
        mFinalBoneMatrices[index] = globalTransformation * offset;
    }

    for (int i = 0; i < pNode->mChildrenCount; i++)
        calculateBoneTransform(&pNode->mChildren[i], globalTransformation);
}