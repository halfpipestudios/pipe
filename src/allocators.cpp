#include "allocators.h"

/*----------------------------------*/
/*          Memory Frame            */
/*----------------------------------*/

void MemoryFrame::BeginFrame() {
    this->address = (u64)MemoryManager::Get()->GetDoubleStack()->top;
}

void MemoryFrame::EndFrame() {
   MemoryManager::Get()->GetDoubleStack()->top = (u8 *)address; 
}

