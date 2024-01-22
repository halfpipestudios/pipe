#ifndef _LEVITATION_SPELL_SYS_H_
#define _LEVITATION_SPELL_SYS_H_

template <typename EM>
struct LevitationSpellSys {

    void Update(EM& em, Level *level, Vec3 cameraPos, f32 gameTime, f32 dt);
    void Render(EM& em);

};

#endif
