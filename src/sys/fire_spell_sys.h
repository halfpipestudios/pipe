#ifndef _FIRE_SPELL_SYS_
#define _FIRE_SPELL_SYS_

template <typename EM>
struct FireSpellSys {
    
    void Update(EM& em, Level *level, Vec3 cameraPos, f32 gameTime, f32 dt);
    void Render(EM& em);

};


#endif
