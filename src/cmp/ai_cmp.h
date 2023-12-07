#ifndef _AI_CMP_H_
#define _AI_CMP_H_

#include "../steering_behaviors.h"

struct BehaviorTree;

struct AiCMP : CMP<AiCMP> {

    SteeringBehavior behavior { STEERING_BEHAVIOR_ARRIVE };
    f32 timeToTarget          { 0 };
    f32 arrivalRadii          { 0 };
    bool active               { true };
    BehaviorTree *bhTree      { nullptr };

    void Initialize(SteeringBehavior behavior_, f32 timeToTarget_, f32 arrivalRadii_,
                    bool active_, BehaviorTree *bhTree_ = nullptr) {

        behavior = behavior_;
        timeToTarget = timeToTarget_;
        arrivalRadii = arrivalRadii_;
        bhTree = bhTree_; 
        active = active_;
    }

};

#endif // _AI_CMP_H_
