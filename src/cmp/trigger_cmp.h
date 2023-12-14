#ifndef _TRIGGER_CMP_H_
#define _TRIGGER_CMP_H_

struct TriggerCMP : CMP<TriggerCMP> {
    Cylinder collider  {};
    SlotmapKey whoTriggerThis {};
    void Initialize(SlotmapKey who, Cylinder collider) {
        this->collider = collider;
        whoTriggerThis = who;
    }
};

#endif
