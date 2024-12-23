#include "cmp/ai_cmp.h"
#include "cmp/animation_cmp.h"
#include "cmp/base_cmp.h"
#include "cmp/collision_cmp.h"
#include "cmp/door_cmp.h"
#include "cmp/enemy_cmp.h"
#include "cmp/fire_spell_cmp.h"
#include "cmp/gem_cmp.h"
#include "cmp/graphics_cmp.h"
#include "cmp/input_cmp.h"
#include "cmp/light_cmp.h"
#include "cmp/material_cmp.h"
#include "cmp/movable_box_cmp.h"
#include "cmp/moving_platform_cmp.h"
#include "cmp/particle_cmp.h"
#include "cmp/physics_cmp.h"
#include "cmp/player_cmp.h"
#include "cmp/pressure_plate_cmp.h"
#include "cmp/transform_cmp.h"
#include "cmp/trigger_cmp.h"
#include "tokenizer.h"

static void AddTypesToEntityManager() {
    EntityManager::Get()->AddComponentType<AiCMP>();
    EntityManager::Get()->AddComponentType<AnimationCMP>();
    EntityManager::Get()->AddComponentType<CollisionCMP>();
    EntityManager::Get()->AddComponentType<DoorCMP>();
    EntityManager::Get()->AddComponentType<EnemyCMP>();
    EntityManager::Get()->AddComponentType<FireSpellCMP>();
    EntityManager::Get()->AddComponentType<GemCMP>();
    EntityManager::Get()->AddComponentType<GraphicsCMP>();
    EntityManager::Get()->AddComponentType<InputCMP>();
    EntityManager::Get()->AddComponentType<LightCMP>();
    EntityManager::Get()->AddComponentType<MaterialCMP>();
    EntityManager::Get()->AddComponentType<MovableBoxCMP>();
    EntityManager::Get()->AddComponentType<MovingPlatformCMP>();
    EntityManager::Get()->AddComponentType<ParticleCMP>();
    EntityManager::Get()->AddComponentType<PhysicsCMP>();
    EntityManager::Get()->AddComponentType<PlayerCMP>();
    EntityManager::Get()->AddComponentType<PressurePlateCMP>();
    EntityManager::Get()->AddComponentType<TransformCMP>();
    EntityManager::Get()->AddComponentType<TriggerCMP>();
}

static CMPBase *CreateCMPFromNextToken(Tokenizer *t, Entity_ *entity) {
    Tokenizer tmp = *t;
    Token token;
    tmp.NextToken(&token);
    if(token.Contains("ai")) {
        return EntityManager::Get()->AddComponent<AiCMP>(entity->key);
    }
    else if(token.Contains("player_animation")) {
        return EntityManager::Get()->AddComponent<AnimationCMP>(entity->key);
    }
    else if(token.Contains("collision")) {
        return EntityManager::Get()->AddComponent<CollisionCMP>(entity->key);
    }
    else if(token.Contains("door")) {
        return EntityManager::Get()->AddComponent<DoorCMP>(entity->key);
    }
    else if(token.Contains("enemy")) {
        return EntityManager::Get()->AddComponent<EnemyCMP>(entity->key);
    }
    else if(token.Contains("fire_spell")) {
        return EntityManager::Get()->AddComponent<FireSpellCMP>(entity->key);
    }
    else if(token.Contains("gem")) {
        return EntityManager::Get()->AddComponent<GemCMP>(entity->key);
    }
    else if(token.Contains("graphics")) {
        return EntityManager::Get()->AddComponent<GraphicsCMP>(entity->key);
    }
    else if(token.Contains("input")) {
        return EntityManager::Get()->AddComponent<InputCMP>(entity->key);
    }
    else if(token.Contains("light")) {
        return EntityManager::Get()->AddComponent<LightCMP>(entity->key);
    }
    else if(token.Contains("material")) {
        return EntityManager::Get()->AddComponent<MaterialCMP>(entity->key);
    }
    else if(token.Contains("movable_box")) {
        return EntityManager::Get()->AddComponent<MovableBoxCMP>(entity->key);
    }
    else if(token.Contains("moving_platform")) {
        return EntityManager::Get()->AddComponent<MovingPlatformCMP>(entity->key);
    }
    else if(token.Contains("particle")) {
        return EntityManager::Get()->AddComponent<ParticleCMP>(entity->key);
    }
    else if(token.Contains("physics")) {
        return EntityManager::Get()->AddComponent<PhysicsCMP>(entity->key);
    }
    else if(token.Contains("player")) {
        return EntityManager::Get()->AddComponent<PlayerCMP>(entity->key);
    }
    else if(token.Contains("pressure_plate")) {
        return EntityManager::Get()->AddComponent<PressurePlateCMP>(entity->key);
    }
    else if(token.Contains("transform")) {
        return EntityManager::Get()->AddComponent<TransformCMP>(entity->key);
    }
    else if(token.Contains("trigger")) {
        return EntityManager::Get()->AddComponent<TriggerCMP>(entity->key);
    }
    ASSERT(!"cmp factory invalid code path");
    return nullptr;
}