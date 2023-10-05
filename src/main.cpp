#include <stdio.h>

#include "platform_selector.h"

int main() {
    
    Platform *platform = PlatformSelector::Get();
    platform->Initialize();

    Input *input = platform->GetInput();
    
    while(platform->IsRunning()) {
        platform->PollEvents();

        if(input->MouseIsPress(MOUSE_BUTTON_L)) printf("mouse down\n");
        if(input->MouseJustPress(MOUSE_BUTTON_M)) printf("mouse just down\n");
        if(input->MouseJustUp(MOUSE_BUTTON_R)) printf("mouse just up\n");
        if(input->KeyIsPress(KEY_Q)) printf("q down\n");
        if(input->KeyJustPress(KEY_W)) printf("w just down\n");
        if(input->KeyJustUp(KEY_E)) printf("e just up\n");

        Sleep(0.016);
    }

    platform->Terminate();

    printf("hello Pipe Engine\n");

    return 0;
}
