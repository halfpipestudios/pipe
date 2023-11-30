#include "behavior_tree.h"
#include "memory_manager.h"
#include "steering_behaviors.h"

#include "components.h"

// BehaviorTree -------------------------------------------------------------
void BehaviorTree::Initialize(size_t bufferSize_) {
    bufferSize = bufferSize_;
    buffer = (u8 *)MemoryManager::Get()->AllocStaticMemory(bufferSize, 1);
    current_ptr = buffer + bufferSize;
}


BehaviorStatus BehaviorTree::run(BehaviorNodeContex *contx) {
    if(nodes.size()) {
        return nodes.back()->run(contx);
    }
    return BEHAVIOR_FAIL;
}


// BehaviorNodes -------------------------------------------------------------

// BehaviorArrive
BehaviorArrive::BehaviorArrive(Vec3 t) : target(t) {}


BehaviorStatus BehaviorArrive::run(BehaviorNodeContex *contx) {
    Steering steering = Arrive(contx->aiComp, contx->phyComp, target, contx->aiComp->timeToTarget);
    contx->phyComp->physics.acc += steering.linear;
    contx->phyComp->physics.angularVel += steering.angular;
    if(steering == Steering{}) {
        return BEHAVIOR_SUCCESS;
    }
    return BEHAVIOR_RUNNING;
}


// BehaviorSequence
BehaviorSequence::BehaviorSequence(std::initializer_list<BehaviorNode *> ls) : nodes(ls) {}


BehaviorStatus BehaviorSequence::run(BehaviorNodeContex *contx) {
    if(currentNode == nodes.end()) {
        resetSequence();
    }

    BehaviorStatus status = (*currentNode)->run(contx);

    switch(status) {
        case BEHAVIOR_FAIL:    { resetSequence(); return BEHAVIOR_FAIL; }
        case BEHAVIOR_RUNNING: { return BEHAVIOR_RUNNING; }
        case BEHAVIOR_SUCCESS: { 
            ++currentNode;
            if(currentNode == nodes.end()) { resetSequence(); return BEHAVIOR_SUCCESS; }
        }
    }
    return BEHAVIOR_RUNNING;
}


// BehaviorSelector
BehaviorSelector::BehaviorSelector(std::initializer_list<BehaviorNode *> ls) : nodes(ls) {}

BehaviorStatus BehaviorSelector::run(BehaviorNodeContex *contx) {
    // TODO: ....
    return {};
}
