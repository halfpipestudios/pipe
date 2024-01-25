#include "door_sys.h"

template <typename EM>
bool DoorSys<EM>::TestBoxPressurePlate(EM& em, SlotmapKey plate, SlotmapKey box) {
    PressurePlateCMP *platePre = em.GetComponent<PressurePlateCMP>(plate);
    CollisionCMP *boxCol = em.GetComponent<CollisionCMP>(box);
    GJK gjk;
    CollisionData colData = gjk.IntersectFast(&platePre->hull,
                                              &boxCol->poly3D.convexHull);
    return colData.hasCollision;
}

template <typename EM>
void DoorSys<EM>::Update(EM& em, f32 dt) {

    auto& doors = em.GetComponents<DoorCMP>();
    auto& boxs = em.GetComponents<MovableBoxCMP>();

    for(i32 i = 0; i < doors.size; ++i) {
        DoorCMP *door = &doors[i];
        CollisionCMP *doorCol = em.GetComponent<CollisionCMP>(door->entityKey);
        GraphicsCMP *doorGra = em.GetComponent<GraphicsCMP>(door->entityKey);

        i32 platesPressed = 0;

        for(i32 plateIndex = 0; plateIndex < door->platesCount; ++plateIndex) {
            for(i32 boxIndex = 0; boxIndex < boxs.size; ++boxIndex) {
                if(TestBoxPressurePlate(em, door->plates[plateIndex], boxs[boxIndex].entityKey)) {
                    platesPressed++;
                    break;
                }
            }
        }
        
        door->open = platesPressed == door->platesCount;

        doorCol->active = !door->open;
        doorGra->active = !door->open;
    }


}
