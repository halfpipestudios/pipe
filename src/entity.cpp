#include "entity.h"

#include "entity_manager.h"
#include "cmp_factory.h"

void Entity_::Serialize(Serializer *s) {
    WriteBeginObject(s, "entity");
    
    Write(s, "name", name);
    Write(s, "num_components", (i32)componentsIds.size);
    
    WriteBeginArray(s, "components");
    for(u32 i = 0; i < componentsIds.size; ++i) {
        
        CMPBase *cmp = EntityManager::Get()->GetComponentBasePtrByCmpID(this, componentsIds[i]);
        cmp->Serialize(s);
    }
    WriteEndArray(s);

    WriteEndObject(s);
}

void Entity_::Deserialize(Tokenizer *t) {
    ReadBeginObject(t, "entity");
    
    i32 numComponents = 0;
    Read(t, "name", name, MAX_ENTITY_NAME);
    Read(t, "num_components", &numComponents);
    
    ReadBeginArray(t, "components");
    for(u32 i = 0; i < numComponents; ++i) {
        CMPBase *cmp = CreateCMPFromNextToken(t, this);
        cmp->Deserialize(t);
    }
    ReadEndArray(t);

    ReadEndObject(t);
}
