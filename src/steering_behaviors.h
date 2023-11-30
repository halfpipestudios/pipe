#ifndef _STEERING_BEHAVIORS_H_
#define _STEERING_BEHAVIORS_H_

#include "common.h"
#include "math.h"

struct AIComponent;
struct PhysicsComponent;

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

Steering  Face(AIComponent *aiComp, PhysicsComponent *phyComp, Vec3 target, f32 timeToTarget);
Steering  Seek(AIComponent *aiComp, PhysicsComponent *phyComp, Vec3 target, f32 timeToTarget);
Steering  Flee(AIComponent *aiComp, PhysicsComponent *phyComp, Vec3 target, f32 timeToTarget);
Steering  Arrive(AIComponent *aiComp, PhysicsComponent *phyComp, Vec3 target, f32 timeToTarget);


#endif
