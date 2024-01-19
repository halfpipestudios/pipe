#ifndef _INPUT_SYS_H_
#define _INPUT_SYS_H_

template <typename EM>
struct InputSys {
    void Update(EM& em, Camera *camera, f32 dt);
};

#endif // _INPUT_CMP_H_
