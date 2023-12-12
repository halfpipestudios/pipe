#ifndef _STEERING_BEHAVIORS_H_
#define _STEERING_BEHAVIORS_H_

#include "common.h"
#include "math.h"

struct AiCMP;
struct PhysicsCMP;

// TODO: fix this ...
struct AIBlackBoard {
    Vec3 *target;
};

extern AIBlackBoard gBlackBoard;

enum SteeringBehavior {
    STEERING_BEHAVIOR_FACE,
    STEERING_BEHAVIOR_SEEK,
    STEERING_BEHAVIOR_FLEE,
    STEERING_BEHAVIOR_ARRIVE
};

struct Steering {
    Vec3 linear;
    f32 angular;

    bool operator==(Steering& s) {
        if ((linear == s.linear) && (angular == s.angular)) {
            return true;
        }
        return false;
    }
};

Steering  Face(AiCMP *aiComp, PhysicsCMP *phyComp, Vec3 target, f32 timeToTarget);
Steering  Seek(AiCMP *aiComp, PhysicsCMP *phyComp, Vec3 target, f32 timeToTarget);
Steering  Flee(AiCMP *aiComp, PhysicsCMP *phyComp, Vec3 target, f32 timeToTarget);
Steering  Arrive(AiCMP *aiComp, PhysicsCMP *phyComp, Vec3 target, f32 timeToTarget);


#endif
