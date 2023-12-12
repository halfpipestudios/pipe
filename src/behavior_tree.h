#ifndef _BEHAVIOR_TREE_H_
#define _BEHAVIOR_TREE_H_

#include <initializer_list>

#include "common.h"
#include "algebra.h"

enum BehaviorStatus {
    BEHAVIOR_SUCCESS,
    BEHAVIOR_FAIL,
    BEHAVIOR_RUNNING
};

#define BEHAVIOR_TREE_MAX_NODES 128
#define BEHAVIOR_NODE_MAX_CHILDS 16

struct Entity_;
struct PhysicsCMP;
struct AiCMP;

struct BehaviorNodeContex {
    Entity_ *entity;
    PhysicsCMP *phyComp;
    AiCMP      *aiComp;
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

    void resetSequence() { currentNode = nodes; }

    BehaviorNode *nodes[BEHAVIOR_NODE_MAX_CHILDS];
    u32 nodesCount { 0 };
    BehaviorNode **currentNode { nodes };
};

// TODO: ...
struct BehaviorSelector : BehaviorNode {
    BehaviorSelector(std::initializer_list<BehaviorNode *> ls);
    BehaviorStatus run(BehaviorNodeContex *contx) override;

    BehaviorNode *nodes[BEHAVIOR_NODE_MAX_CHILDS];
    u32 nodesCount { 0 };
    BehaviorNode **currentNode { nodes };
};

struct BehaviorTree {

    void Initialize(size_t bufferSize = 1024);

    BehaviorStatus run(BehaviorNodeContex *contx);

    template <typename NodeType, typename... ParamType>
    BehaviorNode *AddNode(ParamType&&... params);

    size_t bufferSize = 0;
    u8 *buffer = nullptr;
    u8 *current_ptr = nullptr;

    BehaviorNode **nodes { nullptr };
    u32 nodesCount { 0 };

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
    nodes[nodesCount++] = node;
    return node;
}

#endif
