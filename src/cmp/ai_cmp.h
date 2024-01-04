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

    char *SteeringBehaviorToStr() {
        switch(behavior) {
            case STEERING_BEHAVIOR_FACE: return "face";
            case STEERING_BEHAVIOR_SEEK: return "seek";
            case STEERING_BEHAVIOR_FLEE: return "flee";
            case STEERING_BEHAVIOR_ARRIVE: return "arrive";
        }
        
        ASSERT(!"invalid code path");
        return nullptr;
    }

    void Serialize(Serializer *s) override {
        BeginObject(s, "ai");
        Write(s, "behavior", SteeringBehaviorToStr());
        Write(s, "time_to_target", timeToTarget);
        Write(s, "arraival_radii", arrivalRadii);
        Write(s, "active", active);
        EndObject(s);
    };


};

#endif // _AI_CMP_H_
