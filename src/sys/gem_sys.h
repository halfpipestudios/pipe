#ifndef _GEM_SYS_H_
#define _GEM_SYS_H_

struct Level;
struct GemCMP;
struct TransformCMP;
struct CollisionCMP;
struct TriggerCMP;

template <typename EM>
struct GemSys {
    void Update(EM& em, Level *level, f32 dt);

    void GrabGems(Level *level, GemCMP *gem, 
                  TransformCMP *transform, TriggerCMP *trigger,
                  TransformCMP *whoTransform, CollisionCMP *whoCollider,
                  f32 dt);


    void UpdateGem(GemCMP *gem, TransformCMP *transform, TriggerCMP *trigger, f32 dt);



};

#endif // _GEM_SYS_H_
