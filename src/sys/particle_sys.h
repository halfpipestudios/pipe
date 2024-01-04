#ifndef _PARTICLE_SYS_H_
#define _PARTICLE_SYS_H_

template <typename EM>
struct ParticleSys {
    
    void Update(EM& em, Vec3 cameraPos, f32 gameTime, f32 dt);
    void Render(EM& em);

};

#endif
