#pragma once

#include <algorithm>

class TransitionValue
{
public:
    TransitionValue(float initialValue, float transitionFrames) : mCurrentValue(initialValue), mTargetValue(initialValue), mTransitionFrames(transitionFrames), mSpeed(1) {}

    void setTarget(float target)
    {
        mTargetValue = target;
        mSpeed = abs(mTargetValue - mCurrentValue) / mTransitionFrames;
    }

    void setTransitionFrames(float frames)
    {
        mTransitionFrames = frames;
        mSpeed = abs(mTargetValue - mCurrentValue) / frames;
    }

    void update()
    {
        float delta = mTargetValue - mCurrentValue;

        if (std::abs(delta) < mSpeed)
        {
            mCurrentValue = mTargetValue;
        }
        else
        {
            mCurrentValue += (delta > 0) ? mSpeed : -mSpeed;
        }
    }

    float value()
    {
        return mCurrentValue;
    }

private:
    float mCurrentValue;
    float mTargetValue;
    float mTransitionFrames;
    float mSpeed;
};