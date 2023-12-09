#include "asset_manager.h"

void Handle::Initialize(u32 index) {
    ASSERT(IsNull());
    ASSERT(index <= MAX_INDEX);

    static u32 autoMagic = 0;

    if(++autoMagic > MAX_MAGIC) {
        autoMagic = 1;
    }

    this->index = index;
    this->magic = autoMagic;
}
