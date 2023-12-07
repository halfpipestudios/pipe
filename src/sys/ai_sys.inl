#include "ai_sys.h"

template <typename EM>
void AiSys<EM>::Update(EM& em, f32 dt) {

    auto& ais = em.GetComponents<AiCMP>();

    for(i32 i = 0; i < ais.size; ++i) {
        
        AiCMP *ai = &ais[i];
        Entity_ *entity = ai->entity;
        PhysicsCMP *phy = entity->GetComponent<PhysicsCMP>();
        
        if(phy == nullptr) continue;

        /*
        if(ai->bhTree) {
            BehaviorNodeContex_ contx;
            contx.entity = entity;
            contx.phyComp = phy;
            contx.aiComp = ai;
            bhTree->run(&contx);
            return;
        }
        
        Steering steering = {};

        switch(behavior) {
            case STEERING_BEHAVIOR_FACE:   { steering = Face(  this, phyComp, *gBlackBoard.target, timeToTarget); } break;
            case STEERING_BEHAVIOR_SEEK:   { steering = Seek(  this, phyComp, *gBlackBoard.target, timeToTarget); } break;
            case STEERING_BEHAVIOR_FLEE:   { steering = Flee(  this, phyComp, *gBlackBoard.target, timeToTarget); } break;
            case STEERING_BEHAVIOR_ARRIVE: { steering = Arrive(this, phyComp, *gBlackBoard.target, timeToTarget); } break;
        }
        
        phy->physics.acc += steering.linear;
        phy->physics.angularVel += steering.angular;
        */

        
    }

}
