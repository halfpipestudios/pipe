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

    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "ai");
        Write(s, "behavior", behavior);
        Write(s, "time_to_target", timeToTarget);
        Write(s, "arraival_radii", arrivalRadii);
        Write(s, "active", active);
        WriteEndObject(s);
    }

    void Deserialize(Tokenizer *t) override {
        SteeringBehavior behavior_;
        f32 timeToTarget_;
        f32 arrivalRadii_;
        bool active_;

        ReadBeginObject(t, "ai");
        Read(t, "behavior", &(i32)behavior_);
        Read(t, "time_to_target", &timeToTarget_);
        Read(t, "arraival_radii", &arrivalRadii_);
        Read(t, "active", &active_);
        ReadEndObject(t);

        Initialize(behavior_, timeToTarget_, arrivalRadii_, active_, nullptr);
    }


};

#endif // _AI_CMP_H_
