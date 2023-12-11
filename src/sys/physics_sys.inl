#include "physics_sys.h"

template <typename EM>
void PhysicsSys<EM>::PreUpdate(EM& em, f32 dt) {
    auto& physicsComponents = em.GetComponents<PhysicsCMP>();
    for(i32 i = 0; i < physicsComponents.size; ++i) {
        PhysicsCMP *phy = &physicsComponents[i];
        phy->physics.acc = Vec3();
        phy->physics.angularVel = 0;
    }
}


template <typename EM>
void PhysicsSys<EM>::Update(EM& em, f32 dt) {
    
    auto& physicsComponents = em.GetComponents<PhysicsCMP>();

    for(i32 i = 0; i < physicsComponents.size; ++i) {
        PhysicsCMP *phy = &physicsComponents[i];
        Entity_ *entity = phy->entity;

        // NOTE: Apply gravity
        if(!entity->HaveFlag(ENTITY_GROUNDED))
            phy->physics.acc += Vec3(0, -9.8 * 2.5, 0);

        phy->physics.vel += phy->physics.acc * dt;
        
        // TODO: quick fix for the view direction bug
        phy->viewDir += phy->physics.acc * dt;
        f32 viewDammping = powf(0.001f, dt);
        phy->viewDir = phy->viewDir * viewDammping;


        // NOTE: Apply drag
        if(entity->HaveFlag(ENTITY_GROUNDED)) {
            f32 dammping = powf(0.001f, dt);
            phy->physics.vel = phy->physics.vel * dammping;
        }
        else {
            f32 dammping = powf(0.5f, dt);
            phy->physics.vel = phy->physics.vel * dammping;
        }
        
        phy->physics.pos += phy->physics.vel * dt;
        phy->physics.orientation += phy->physics.angularVel * dt;
    }

}

template <typename EM>
void PhysicsSys<EM>::PostUpdate(EM& em, f32 dt) {
    auto& physicsComponents = em.GetComponents<PhysicsCMP>();
    for(i32 i = 0; i < physicsComponents.size; ++i) {
        PhysicsCMP *phy = &physicsComponents[i];
        phy->lastPhysics = phy->physics;
    }
}
