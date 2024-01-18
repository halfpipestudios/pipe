#include "cmp/ai_cmp.h"
#include "cmp/animation_cmp.h"
#include "cmp/base_cmp.h"
#include "cmp/collision_cmp.h"
#include "cmp/enemy_cmp.h"
#include "cmp/fire_spell_cmp.h"
#include "cmp/gem_cmp.h"
#include "cmp/graphics_cmp.h"
#include "cmp/input_cmp.h"
#include "cmp/moving_platform_cmp.h"
#include "cmp/particle_cmp.h"
#include "cmp/physics_cmp.h"
#include "cmp/player_cmp.h"
#include "cmp/transform_cmp.h"
#include "cmp/trigger_cmp.h"
#include "tokenizer.h"

static void AddTypesToEntityManager() {
    EntityManager::Get()->AddComponentType<AiCMP>();
    EntityManager::Get()->AddComponentType<AnimationCMP>();
    EntityManager::Get()->AddComponentType<CollisionCMP>();
    EntityManager::Get()->AddComponentType<EnemyCMP>();
    EntityManager::Get()->AddComponentType<FireSpellCMP>();
    EntityManager::Get()->AddComponentType<GemCMP>();
    EntityManager::Get()->AddComponentType<GraphicsCMP>();
    EntityManager::Get()->AddComponentType<InputCMP>();
    EntityManager::Get()->AddComponentType<MovingPlatformCMP>();
    EntityManager::Get()->AddComponentType<ParticleCMP>();
    EntityManager::Get()->AddComponentType<PhysicsCMP>();
    EntityManager::Get()->AddComponentType<PlayerCMP>();
    EntityManager::Get()->AddComponentType<TransformCMP>();
    EntityManager::Get()->AddComponentType<TriggerCMP>();
}

static CMPBase *CreateCMPFromNextToken(Tokenizer *t, Entity_ *entity) {
    Tokenizer tmp = *t;
    Token token;
    tmp.NextToken(&token);
    if(token.Contains("collision")) {
        return EntityManager::Get()->AddComponent<CollisionCMP>(entity->key);
    }
    else if(token.Contains("graphics")) {
        return EntityManager::Get()->AddComponent<GraphicsCMP>(entity->key);
    }
    else if(token.Contains("transform")) {
        return EntityManager::Get()->AddComponent<TransformCMP>(entity->key);
    }
    ASSERT(!"cmp factory invalid code path");
    return nullptr;
}