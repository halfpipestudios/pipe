#include "light_sys.h"
#include "graphics_manager.h"
#include "memory_manager.h"
#include "mgr/shadowmap_manager.h"

template <typename EM>
void LightSys<EM>::Update(EM& em, Camera *camera) {
    
    auto& lightComponents = em.GetComponents<LightCMP>();

    if(lightComponents.size <= 0){
        GraphicsManager::Get()->UpdateLights(camera->pos, SHADOW_MAP_FAR_PLANE, nullptr, 0);
        return;
    }

    // TODO: we do not need this (it is temp)
    ASSERT(lightComponents.size <= MAX_LIGHTS_COUNT);

    MemoryManager::Get()->BeginTemporalMemory();
    Light *lightsToRender = (Light *)MemoryManager::Get()->AllocTemporalMemory(lightComponents.size * sizeof(Light), 8);
    i32 count = 0;
    
    ShadowMap *shadowMapsToRender = (ShadowMap *)MemoryManager::Get()->AllocTemporalMemory(lightComponents.size * sizeof(ShadowMap), 8);
    i32 shadowMapCount = 0;

    for(i32 i = 0; i < lightComponents.size; i++) {
        LightCMP *lig = &lightComponents[i];

        SlotmapKey entity = lig->entityKey;
        TransformCMP *trans = em.GetComponent<TransformCMP>(entity);

        if(trans != nullptr) {
            lig->pos = trans->pos;
        }

        Light *light = lightsToRender + i;
        light->type = lig->type;
        light->pos = lig->pos;
        light->dir = lig->dir;
        light->ambient = lig->ambient;
        light->diffuse = lig->diffuse;
        light->specular = lig->specular;
        light->constant = lig->constant;
        light->linear = lig->linear;
        light->quadratic = lig->quadratic;
        light->cutOff = lig->cutOff;
        light->outerCutOff = lig->outerCutOff;
 
        if(lig->type == LIGHT_TYPE_POINT && lig->haveSahdowMap) {
            shadowMapsToRender[shadowMapCount] = *ShadowMapManager::Get()->Dereference(lig->shadowMap);
            light->shadowMapIndex = shadowMapCount;
            shadowMapCount++;
        }

        count++;
        // TODO: only get the components closer to the player
        // for now we are going to get all 
    }

    GraphicsManager::Get()->BindShadowMaps(shadowMapsToRender, shadowMapCount);
    GraphicsManager::Get()->UpdateLights(camera->pos, SHADOW_MAP_FAR_PLANE, lightsToRender, count);

    MemoryManager::Get()->EndTemporalMemory();

}
