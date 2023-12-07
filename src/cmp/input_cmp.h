#ifndef _INPUT_CMP_H_
#define _INPUT_CMP_H_

struct Input;
struct Camera;

struct InputCMP : CMP<InputCMP> {

    void Initialize(Input *input_, Camera *camera_) {
        input = input_;
        camera = camera_;
    }

    Input *input;
    Camera *camera;
};

#endif _INPUT_CMP_H_
