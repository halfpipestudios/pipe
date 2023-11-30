#ifndef _BEHAVIOR_TREE_H_
#define _BEHAVIOR_TREE_H_

#include <memory>
#include <vector>
#include <initializer_list>

#include "common.h"
#include "math.h"

enum BehaviorStatus {
    BEHAVIOR_SUCCESS,
    BEHAVIOR_FAIL,
    BEHAVIOR_RUNNING
};

struct Entity;
struct PhysicsComponent;
struct AIComponent;

struct BehaviorNodeContex {
    Entity *entity;
    PhysicsComponent *phyComp;
    AIComponent      *aiComp;
};

struct BehaviorNode {
    BehaviorNode()          = default;
    virtual ~BehaviorNode() = default;
    
    virtual BehaviorStatus run(BehaviorNodeContex *contx) = 0; 
};

struct BehaviorArrive : BehaviorNode {
    BehaviorArrive(Vec3 t);
    BehaviorStatus run(BehaviorNodeContex *contx) override;

    Vec3 target;
};

struct BehaviorSequence : BehaviorNode {
    BehaviorSequence(std::initializer_list<BehaviorNode *> ls);
    BehaviorStatus run(BehaviorNodeContex *contx) override;

    void resetSequence() { currentNode = nodes.begin(); }

    std::vector<BehaviorNode *>nodes                  {};
    std::vector<BehaviorNode *>::iterator currentNode { nodes.begin() };
};

struct BehaviorSelector : BehaviorNode {
    BehaviorSelector(std::initializer_list<BehaviorNode *> ls);
    BehaviorStatus run(BehaviorNodeContex *contx) override;

    std::vector<BehaviorNode *>nodes                  {};
    std::vector<BehaviorNode *>::iterator currentNode { nodes.begin() };
};

struct BehaviorTree {

    void Initialize(size_t bufferSize = 1024);

    BehaviorStatus run(BehaviorNodeContex *contx);

    template <typename NodeType, typename... ParamType>
    BehaviorNode *AddNode(ParamType&&... params);

    size_t bufferSize = 0;
    u8 *buffer = nullptr;
    u8 *current_ptr = nullptr;

    std::vector<BehaviorNode *> nodes;

};


template <typename NodeType, typename... ParamType>
BehaviorNode *BehaviorTree::AddNode(ParamType&&... params) {
    current_ptr -= sizeof(NodeType);

    if(current_ptr < buffer) {
        printf("no more memory in the behavior tree buffer\n");
        ASSERT(!"INVALID_CODE_PATH");
        return nullptr;
    }

    NodeType *node = new(current_ptr) NodeType{std::forward<ParamType>(params)...};
    nodes.push_back(node);
    return node;
}

#endif
