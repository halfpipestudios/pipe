#include <stdio.h>

#include "platform_selector.h"

Platform *gPlatform = nullptr;

int main() {
    
    gPlatform = PlatformSelector::Get();
    gPlatform->Initialize();

    Input *input = gPlatform->GetInput();
    
    u64 pageSize = 4096;

    void *memory = gPlatform->MemoryReserve(pageSize*2);
    gPlatform->MemoryCommit(memory, pageSize);
    gPlatform->MemoryCommit((u8 *)memory+pageSize, pageSize);
    
    u32 *array = (u32 *)memory;
    for(u32 i = 0; i < 10; ++i) {
        array[i] = i;
    }

    for(u32 i = 0; i < 10; ++i) {
        printf("i) %d\n", array[i]);
    }

    array = (u32 *)((u8 *)memory + pageSize);
    for(u32 i = 0; i < 10; ++i) {
        array[i] = i;
    }

    for(u32 i = 0; i < 10; ++i) {
        printf("i) %d\n", array[i]);
    }

    gPlatform->MemoryRelease(memory, pageSize*2);

    while(gPlatform->IsRunning()) {
        gPlatform->PollEvents();

        if(input->MouseIsPress(MOUSE_BUTTON_L)) printf("mouse down\n");
        if(input->MouseJustPress(MOUSE_BUTTON_M)) printf("mouse just down\n");
        if(input->MouseJustUp(MOUSE_BUTTON_R)) printf("mouse just up\n");
        if(input->KeyIsPress(KEY_Q)) printf("q down\n");
        if(input->KeyJustPress(KEY_W)) printf("w just down\n");
        if(input->KeyJustUp(KEY_E)) printf("e just up\n");

        Sleep(0.016);
    }

    gPlatform->Terminate();

    printf("hello Pipe Engine\n");

    return 0;
}
