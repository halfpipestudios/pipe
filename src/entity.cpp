#include "entity.h"

#include "entity_manager.h"

void Entity_::Serialize(Serializer *s) {
    BeginObject(s, "entity");
    
    Write(s, "name", name);
    Write(s, "num_components", (i32)componentsIds.size);
    
    BeginArray(s, "components");
    for(u32 i = 0; i < componentsIds.size; ++i) {
        
        CMPBase *cmp = EntityManager::Get()->GetComponentBasePtrByCmpID(this, componentsIds[i]);
        cmp->Serialize(s);
    }
    EndArray(s);

    EndObject(s);
}

