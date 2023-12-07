#include "steering_behaviors.h"

#include "cmp/physics_cmp.h"
#include "cmp/ai_cmp.h"

#include <stdio.h>

AIBlackBoard gBlackBoard;

static void AdjustAngle(f32& angle) {
    while(angle >  (f32)PI) angle -= (f32)(2*PI);
    while(angle < -(f32)PI) angle += (f32)(2*PI);
}

static f32 maxAcceleration = 25.0f;
static f32 maxAngularVel = 3.0f*(f32)PI;

static f32 Vec2ToAngle(Vec2 v) {
    f32 angle = atan2f(v.y, v.x);
    if(angle < 0) angle += (f32)(2*PI);
    return angle;
}

static Vec2 AngleToVec2(f32 a) {
    Vec2 v;
    v.x = cosf(a);
    v.y = sinf(a);
    return v;
}

f32 MachAngle(f32 currentAngle, f32 targetAngle, f32 timeToTarget) {
    f32 angularDist = targetAngle - currentAngle;
    AdjustAngle(angularDist);
    f32 angularVel = angularDist / timeToTarget;
    return angularVel;
}

Steering Face(AiCMP *aiComp, PhysicsCMP *phyComp, Vec3 target, f32 timeToTarget) {
    Steering steering;

    // get the direction to align with
    Vec3 dir = target - phyComp->physics.pos;
    dir.Normalize();

    // get the angle of this direction
    f32 currentOrientation = phyComp->physics.orientation;
    f32 targetOrientation = Vec2ToAngle({dir.x, dir.z});
    f32 angularVel = MachAngle(currentOrientation, targetOrientation, timeToTarget);
    
    steering.linear = Vec3();
    steering.angular = CLAMP(angularVel, -maxAngularVel, maxAngularVel);

    return steering;
}

Steering Seek(AiCMP *aiComp, PhysicsCMP *phyComp, Vec3 target, f32 timeToTarget) {
    Steering steering;

    steering = Face(aiComp, phyComp, target, timeToTarget);

    f32 angularVelSize = fabsf(steering.angular);
    f32 acc = maxAcceleration / (1 + angularVelSize);
    acc = CLAMP(acc / timeToTarget, -maxAcceleration, maxAcceleration);

    // Get the direction to travel
    Vec2 dir = AngleToVec2(phyComp->physics.orientation);
    steering.linear.x = dir.x;
    steering.linear.y = 0.0f;
    steering.linear.z = dir.y;

    //Give full acceleration along this direction
    steering.linear.Normalize();
    steering.linear *= acc;

    return steering;
}

Steering Flee(AiCMP *aiComp, PhysicsCMP *phyComp, Vec3 target, f32 timeToTarget) {
    Vec3 oppositeTarget = (phyComp->physics.pos*2.0f) - target;
    return Seek(aiComp, phyComp, oppositeTarget,  timeToTarget);
}


Steering  Arrive(AiCMP *aiComp, PhysicsCMP *phyComp, Vec3 target, f32 timeToTarget) {
    Steering steering;

    steering = Face(aiComp, phyComp, target, timeToTarget);

    // Get the direction to travel
    Vec2 dir = AngleToVec2(phyComp->physics.orientation);
    steering.linear.x = dir.x;
    steering.linear.y = 0.0f;
    steering.linear.z = dir.y;

    f32 angularVelSize = fabsf(steering.angular);
    f32 acc = maxAcceleration / (1 + angularVelSize);

    //Give full acceleration along this direction
    steering.linear.Normalize();
    steering.linear *= acc / timeToTarget;

    Vec3 pos = {phyComp->physics.pos.x, 0.0f, phyComp->physics.pos.z};
    f32 distToTargte = (target - pos).Len();
    if(distToTargte <= aiComp->arrivalRadii) {
        steering.linear = Vec3();
        steering.angular = 0;
    }

    return steering;
}

