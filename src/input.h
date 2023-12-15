#ifndef _INPUT_H_
#define _INPUT_H_

#include "common.h"

struct InputState {
    bool keys[349];
    bool mouseButtons[3];
    i32 mouseX;
    i32 mouseY;
    i32 wheelDelta;
    bool joyButtons[12];
    f32 leftStickX;
    f32 leftStickY;
    f32 rightStickX;
    f32 rightStickY;
    
    // NOTE: Editor input
    char text[4];
    u32 textSize;

    bool kBackspace;
    bool kRightArrow;
    bool kLeftArrow;
};

struct Input {
    InputState state[2];

    bool MouseIsPress(u32 button);
    bool MouseJustPress(u32 button);
    bool MouseJustUp(u32 button);

    i32 MouseX();
    i32 MouseY();
    i32 MouseLastX();
    i32 MouseLastY();
    
    bool KeyIsPress(u32 kcode);
    bool KeyJustPress(u32 kcode);
    bool KeyJustUp(u32 kcode);

    bool JoystickIsPress(u32 button);
    bool JoystickJustPress(u32 button);
    bool JoystickJustUp(u32 button);
};

#define JOYSTICK_BUTTON_UP     0
#define JOYSTICK_BUTTON_DOWN   1
#define JOYSTICK_BUTTON_LEFT   2
#define JOYSTICK_BUTTON_RIGHT  3
#define JOYSTICK_BUTTON_START  4
#define JOYSTICK_BUTTON_BACK   5
#define JOYSTICK_BUTTON_A      6
#define JOYSTICK_BUTTON_B      7
#define JOYSTICK_BUTTON_X      8
#define JOYSTICK_BUTTON_Y      9
#define JOYSTICK_LEFT_TRIGGER  10
#define JOYSTICK_RIGHT_TRIGGER 11

#define MOUSE_BUTTON_L 0
#define MOUSE_BUTTON_M 1
#define MOUSE_BUTTON_R 2

#define KEY_BACKESPACE	 0x08
#define KEY_ESCAPE      0x1B
#define KEY_0  0x30 
#define KEY_1  0x31 
#define KEY_2  0x32 
#define KEY_3  0x33 
#define KEY_4  0x34 
#define KEY_5  0x35 
#define KEY_6  0x36 
#define KEY_7  0x37 
#define KEY_8  0x38 
#define KEY_9  0x39 
#define KEY_A  0x41 
#define KEY_B  0x42 
#define KEY_C  0x43 
#define KEY_D  0x44 
#define KEY_E  0x45 
#define KEY_F  0x46 
#define KEY_G  0x47 
#define KEY_H  0x48 
#define KEY_I  0x49 
#define KEY_J  0x4A 
#define KEY_K  0x4B 
#define KEY_L  0x4C 
#define KEY_M  0x4D 
#define KEY_N  0x4E 
#define KEY_O  0x4F 
#define KEY_P  0x50 
#define KEY_Q  0x51 
#define KEY_R  0x52 
#define KEY_S  0x53 
#define KEY_T  0x54 
#define KEY_U  0x55 
#define KEY_V  0x56 
#define KEY_W  0x57 
#define KEY_X  0x58 
#define KEY_Y  0x59 
#define KEY_Z  0x5A 
#define KEY_NUMPAD0  0x60	    
#define KEY_NUMPAD1  0x61	    
#define KEY_NUMPAD2  0x62	    
#define KEY_NUMPAD3  0x63	    
#define KEY_NUMPAD4  0x64	    
#define KEY_NUMPAD5  0x65	    
#define KEY_NUMPAD6  0x66	    
#define KEY_NUMPAD7  0x67	    
#define KEY_NUMPAD8  0x68	    
#define KEY_NUMPAD9  0x69	    
#define KEY_RETURN   0x0D
#define KEY_SPACE    0x20
#define KEY_TAB      0x09
#define KEY_CONTROL  0x11
#define KEY_SHIFT    0x10
#define KEY_ALT      0x12
#define KEY_CAPS     0x14
#define KEY_LEFT     0x25
#define KEY_UP       0x26
#define KEY_RIGHT    0x27
#define KEY_DOWN     0x28

#endif
