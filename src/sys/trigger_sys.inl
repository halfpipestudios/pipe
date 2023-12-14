#ifndef _TRIGGER_SYS_INL_
#define _TRIGGER_SYS_INL_

template <typename EM>
struct TriggerSys {
    void Update(EM& em, f32 dt);

};

template <typename EM>
void TriggerSys<EM>::Update(EM& em, f32 dt) {
    
    auto& triggers = em.GetComponents<TriggerCMP>();

    for(i32 i = 0; i < triggers.size; ++i) {
        TriggerCMP *trigger = &triggers[i];
        TransformCMP *transform = em.GetComponent<TransformCMP>(trigger->entityKey);
        if(transform == nullptr) continue;
        trigger->collider.c = transform->pos;
    }
}

#endif
