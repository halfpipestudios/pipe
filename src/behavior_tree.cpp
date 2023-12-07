#include "behavior_tree.h"
#include "memory_manager.h"
#include "steering_behaviors.h"

#include "cmp/physics_cmp.h"
#include "cmp/ai_cmp.h"

#include <initializer_list>
#include <memory>

// BehaviorTree -------------------------------------------------------------
void BehaviorTree::Initialize(size_t bufferSize_) {
    bufferSize = bufferSize_;
    buffer = (u8 *)MemoryManager::Get()->AllocStaticMemory(bufferSize, 1);
    current_ptr = buffer + bufferSize;

    nodes = (BehaviorNode **)MemoryManager::Get()->AllocStaticMemory(BEHAVIOR_TREE_MAX_NODES * sizeof(BehaviorNode *), 1);
}


BehaviorStatus BehaviorTree::run(BehaviorNodeContex *contx) {
    if(nodesCount) {
        return nodes[nodesCount - 1]->run(contx);
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
BehaviorSequence::BehaviorSequence(std::initializer_list<BehaviorNode *> ls) {
    ASSERT(ls.size() <= BEHAVIOR_NODE_MAX_CHILDS);
    nodesCount = ls.size();
    memcpy(nodes, ls.begin(), sizeof(BehaviorNode *) * ls.size());
}


BehaviorStatus BehaviorSequence::run(BehaviorNodeContex *contx) {
    if(currentNode == (nodes + nodesCount)) {
        resetSequence();
    }

    BehaviorStatus status = (*currentNode)->run(contx);

    switch(status) {
        case BEHAVIOR_FAIL:    { resetSequence(); return BEHAVIOR_FAIL; }
        case BEHAVIOR_RUNNING: { return BEHAVIOR_RUNNING; }
        case BEHAVIOR_SUCCESS: { 
            ++currentNode;
            if(currentNode == (nodes + nodesCount)) { resetSequence(); return BEHAVIOR_SUCCESS; }
        }
    }
    return BEHAVIOR_RUNNING;
}


// BehaviorSelector
BehaviorSelector::BehaviorSelector(std::initializer_list<BehaviorNode *> ls) {
    ASSERT(ls.size() <= BEHAVIOR_NODE_MAX_CHILDS);
    nodesCount = ls.size();
    memcpy(nodes, ls.begin(), sizeof(BehaviorNode *) * ls.size());
}

BehaviorStatus BehaviorSelector::run(BehaviorNodeContex *contx) {
    // TODO: ....
    return {};
}
