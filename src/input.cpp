#include "input.h"

bool Input::MouseIsPress(u32 button) {
    return state[0].mouseButtons[button];
}

bool Input::MouseJustPress(u32 button) {
    return state[0].mouseButtons[button] && !state[1].mouseButtons[button];
}

bool Input::MouseJustUp(u32 button) {
    return !state[0].mouseButtons[button] && state[1].mouseButtons[button];
}

bool Input::KeyIsPress(u32 kcode) {
    return state[0].keys[kcode];
}

bool Input::KeyJustPress(u32 kcode) {
    return state[0].keys[kcode] && !state[1].keys[kcode];
}

bool Input::KeyJustUp(u32 kcode) {
    return !state[0].keys[kcode] && state[1].keys[kcode];
}

bool Input::JoystickIsPress(u32 button) {
    return state[0].joyButtons[button];
}

bool Input::JoystickJustPress(u32 button) {
    return state[0].joyButtons[button] && !state[1].joyButtons[button];
}

bool Input::JoystickJustUp(u32 button) {
    return !state[0].joyButtons[button] && state[1].joyButtons[button];
}

i32 Input::MouseX() {
    return state[0].mouseX;
}

i32 Input::MouseY() {
    return state[0].mouseY;
}

i32 Input::MouseLastX() {
    return state[1].mouseX;
}

i32 Input::MouseLastY() {
    return state[1].mouseY;
}
