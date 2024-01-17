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