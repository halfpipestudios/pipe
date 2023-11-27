#include "steering_behaviors.h"

#include "components.h"
#include <stdio.h>

AIBlackBoard gBlackBoard;

static void AdjustAngle(f32& angle) {
    while(angle >  PI) angle -= 2*PI;
    while(angle < -PI) angle += 2*PI;
}

static f32 maxAcceleration = 50.0f;
static f32 maxAngularVel = 3*PI;

Steering Face(AIComponent *aiComp, PhysicsComponent *phyComp, Vec3 target, f32 timeToTarget) {
    Steering steering;

    steering.linear = Vec3();

    // get the direction to align with
    Vec3 dir = target - phyComp->physics.pos;
    dir.Normalize();

    // get the angle of this direction
    f32 targetOrientation = atan2f(dir.z, dir.x) - PI*0.5;
    if(targetOrientation < 0) targetOrientation += 2*PI;

    f32 angularDist = targetOrientation - phyComp->physics.orientation;
    AdjustAngle(angularDist);

    f32 angularVel = angularDist / timeToTarget;
    steering.angular = CLAMP(angularVel, -maxAngularVel, maxAngularVel);

    return steering;
}

Steering Seek(AIComponent *aiComp, PhysicsComponent *phyComp, Vec3 target, f32 timeToTarget) {
    Steering steering;

    steering = Face(aiComp, phyComp, target, timeToTarget);

    f32 angularVelSize = fabsf(steering.angular);
    
    f32 acc = maxAcceleration / (1 + angularVelSize);
    
    acc = CLAMP(acc / timeToTarget, -maxAcceleration, maxAcceleration);

    // Get the direction to travel
    steering.linear.x = cosf(phyComp->physics.orientation + PI*0.5);
    steering.linear.y = 0.0f;
    steering.linear.z = sinf(phyComp->physics.orientation + PI*0.5);

    //Give full acceleration along this direction
    steering.linear.Normalize();
    steering.linear *= acc;
    

    return steering;
}

Steering Flee(AIComponent *aiComp, PhysicsComponent *phyComp, Vec3 target, f32 timeToTarget) {
    Vec3 oppositeTarget = (phyComp->physics.pos*2.0f) - target;
    return Seek(aiComp, phyComp, oppositeTarget,  timeToTarget);
}

