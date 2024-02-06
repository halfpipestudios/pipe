#ifndef _BEHAVIOR_TREE_H_
#define _BEHAVIOR_TREE_H_

#include <initializer_list>
#include <utility>
#include <memory>

#include "common.h"
#include "algebra.h"

#include "data_structures.h"
#include "serializer.h"
#include "tokenizer.h"

enum BehaviorStatus {
    BEHAVIOR_SUCCESS,
    BEHAVIOR_FAIL,
    BEHAVIOR_RUNNING
};

enum BehaviorNodeType {
    BEHAVIOR_NODE_TYPE_ARRIVE,
    BEHAVIOR_NODE_TYPE_SEQUENCE,
    BEHAVIOR_NODE_TYPE_SELECTOR
};

#define BEHAVIOR_TREE_MAX_NODES 128
#define BEHAVIOR_NODE_MAX_CHILDS 16

struct PhysicsCMP;
struct AiCMP;

struct BehaviorNodeContex {
    SlotmapKey entityKey;
    PhysicsCMP *phyComp;
    AiCMP      *aiComp;
};

struct BehaviorNode : Serializable {
    BehaviorNode()          = default;
    virtual ~BehaviorNode() = default;
    
    virtual BehaviorStatus run(BehaviorNodeContex *contx) = 0; 

    BehaviorNodeType type;

   
};

struct BehaviorArrive : BehaviorNode {
    BehaviorArrive(Vec3 t);
    BehaviorStatus run(BehaviorNodeContex *contx) override;
    Vec3 target;

    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "node");
        Write(s, "type", type);
        Write(s, "target", target);
        WriteEndObject(s);
    }

    void Deserialize(Tokenizer *t) override { }
};

struct BehaviorSequence : BehaviorNode {
    BehaviorSequence(std::initializer_list<BehaviorNode *> ls);
    BehaviorStatus run(BehaviorNodeContex *contx) override;

    void resetSequence() { currentNode = nodes; }

    BehaviorNode *nodes[BEHAVIOR_NODE_MAX_CHILDS];
    u32 nodesCount { 0 };
    BehaviorNode **currentNode { nodes };

    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "node");
        Write(s, "type", type);
        Write(s, "node_count", nodesCount);
        for(i32 i = 0; i < nodesCount; i++) {
            nodes[i]->Serialize(s);
        }
        WriteEndObject(s);
    }

    void Deserialize(Tokenizer *t) override { }
};

// TODO: ...
struct BehaviorSelector : BehaviorNode {
    BehaviorSelector(std::initializer_list<BehaviorNode *> ls);
    BehaviorStatus run(BehaviorNodeContex *contx) override;

    BehaviorNode *nodes[BEHAVIOR_NODE_MAX_CHILDS];
    u32 nodesCount { 0 };
    BehaviorNode **currentNode { nodes };

    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "node");
        Write(s, "type", type);
        Write(s, "node_count", nodesCount);
        for(i32 i = 0; i < nodesCount; i++) {
            nodes[i]->Serialize(s);
        }
        WriteEndObject(s);
    }
    void Deserialize(Tokenizer *t) override { }
};

struct BehaviorTree : Serializable {

    void Initialize();

    BehaviorStatus run(BehaviorNodeContex *contx);

    template <typename NodeType, typename... ParamType>
    BehaviorNode *AddNode(ParamType&&... params);

    size_t bufferSize = 1024;
    u8 buffer[1024];
    u8 *current_ptr = nullptr;

    BehaviorNode *nodes[BEHAVIOR_TREE_MAX_NODES];
    u32 nodesCount { 0 };

    void Serialize(Serializer *s) override;
    void Deserialize(Tokenizer *t) override;
    BehaviorNode *DeserializeNode(Tokenizer *t);

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

BehaviorNode *DeserializeBehaviorNode(Tokenizer *t, BehaviorTree *bhTree);
void DeserializeBehaviorTree(Tokenizer *t, BehaviorTree *bhTree);

#endif
