#ifndef _LIGHT_SYS_H_
#define _LIGHT_SYS_H_

template <typename EM>
struct LightSys {
    void Update(EM& em, Camera *camera);
};

#endif
