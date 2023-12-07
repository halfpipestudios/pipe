#ifndef _GRAPHIC_CMP_H_
#define _GRAPHIC_CMP_H_

#include "graphics.h"

struct GraphicsCMP : CMP<GraphicsCMP> {
    void Initialize(Model model_, Shader shader_) {
        model = model_;
        shader = shader_;
    }

    Model model;
    Shader shader;
};


#endif // _GRAPHIC_CMP_H_
