#ifndef _SHADOW_MAP_BUILDER_SYS_H_
#define _SHADOW_MAP_BUILDER_SYS_H_

template <typename EM>
struct ShadowMapBuilderSys {
    void GenerateSahdowMaps(EM& em, Level *level);
    void RenderScene(EM& em, Map *map);
};

#endif
