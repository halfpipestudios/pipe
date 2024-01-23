#ifndef _LEVITATION_SPELL_SYS_H_
#define _LEVITATION_SPELL_SYS_H_

struct MovableBoxCMP;

template <typename EM>
struct LevitationSpellSys {

    void Update(EM& em, Camera *camera, f32 gameTime, f32 dt);
    void Render(EM& em);


    MovableBoxCMP *selectedBox { nullptr };

};

#endif
