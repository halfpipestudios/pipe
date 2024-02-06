#include "behavior_tree.h"
#include "memory_manager.h"
#include "steering_behaviors.h"

#include "cmp/physics_cmp.h"
#include "cmp/ai_cmp.h"

#include <initializer_list>
#include <memory>

// BehaviorTree -------------------------------------------------------------
void BehaviorTree::Initialize() {
    current_ptr = buffer + bufferSize;
}


BehaviorStatus BehaviorTree::run(BehaviorNodeContex *contx) {
    if(nodesCount) {
        return nodes[nodesCount - 1]->run(contx);
    }
    return BEHAVIOR_FAIL;
}


// BehaviorNodes -------------------------------------------------------------

// BehaviorArrive
BehaviorArrive::BehaviorArrive(Vec3 t) : target(t) {
    type = BEHAVIOR_NODE_TYPE_ARRIVE;
}


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
    type = BEHAVIOR_NODE_TYPE_SEQUENCE;
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
    type = BEHAVIOR_NODE_TYPE_SELECTOR;
    nodesCount = ls.size();
    memcpy(nodes, ls.begin(), sizeof(BehaviorNode *) * ls.size());
}

BehaviorStatus BehaviorSelector::run(BehaviorNodeContex *contx) {
    // TODO: ....
    return {};
}

void BehaviorTree::Serialize(Serializer *s) {
    WriteBeginObject(s, "behavorTree");
    nodes[nodesCount - 1]->Serialize(s);
    WriteEndObject(s);
}

void BehaviorTree::Deserialize(Tokenizer *t) {
    ReadBeginObject(t, "behavorTree");
    DeserializeNode(t);
    ReadEndObject(t);
}


BehaviorNode *BehaviorTree::DeserializeNode(Tokenizer *t) {
    BehaviorNode *node = nullptr;
    BehaviorNodeType type;
    ReadBeginObject(t, "node");
    Read(t, "type", &(i32)type);
    switch(type) {
        case BEHAVIOR_NODE_TYPE_ARRIVE: {

            Vec3 target;
            Read(t, "target", &target);
            node = AddNode<BehaviorArrive>(target);

        } break;
        case BEHAVIOR_NODE_TYPE_SEQUENCE: {

            i32 nodeCount;
            Read(t, "node_count", &nodeCount);
            BehaviorNode *tmpNodes[BEHAVIOR_NODE_MAX_CHILDS];
            for(i32 i = 0; i < nodeCount; i++) {
                tmpNodes[i] = DeserializeNode(t);
            }
            node = AddNode<BehaviorSequence>(std::initializer_list(&tmpNodes[0], &tmpNodes[nodeCount]));

        } break; 
        case BEHAVIOR_NODE_TYPE_SELECTOR: {
            // TODO: ...
        } break;
    }
    ReadEndObject(t);
    ASSERT(node != nullptr);
    return node;
}
