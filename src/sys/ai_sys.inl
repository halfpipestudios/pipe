#include "ai_sys.h"

#include "../behavior_tree.h"

template <typename EM>
void AiSys<EM>::Update(EM& em, f32 dt) {

    auto& ais = em.GetComponents<AiCMP>();

    for(i32 i = 0; i < ais.size; ++i) {
        
        AiCMP *ai = &ais[i];
        PhysicsCMP *phy = em.GetComponent<PhysicsCMP>(ai->entityKey);
        
        if(phy == nullptr) continue;

        BehaviorNodeContex contx;
        contx.entityKey = ai->entityKey;
        contx.phyComp = phy;
        contx.aiComp = ai;
        ai->bhTree.run(&contx);
#if 0 
        Steering steering = {};

        switch(ai->behavior) {
            case STEERING_BEHAVIOR_FACE:   { steering = Face(  ai, phy, *gBlackBoard.target, ai->timeToTarget); } break;
            case STEERING_BEHAVIOR_SEEK:   { steering = Seek(  ai, phy, *gBlackBoard.target, ai->timeToTarget); } break;
            case STEERING_BEHAVIOR_FLEE:   { steering = Flee(  ai, phy, *gBlackBoard.target, ai->timeToTarget); } break;
            case STEERING_BEHAVIOR_ARRIVE: { steering = Arrive(ai, phy, *gBlackBoard.target, ai->timeToTarget); } break;
        }
        
        phy->physics.acc += steering.linear;
        phy->physics.angularVel += steering.angular;
#endif
        
    }

}
