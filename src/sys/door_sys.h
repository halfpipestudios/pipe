#ifndef _DOOR_SYS_H_
#define _DOOR_SYS_H_

template <typename EM>
struct DoorSys {
    void Update(EM& em, f32 dt);
    bool TestBoxPressurePlate(EM& em, SlotmapKey plate, SlotmapKey box);
};

#endif
