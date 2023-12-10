#ifndef _ASSET_MANAGER_H_
#define _ASSET_MANAGER_H_

#include "common.h"
#include "data_structures.h"

struct Handle {

    enum {
        MAX_BITS_INDEX = 16,
        MAX_BITS_MAGIC = 16,

        MAX_INDEX = (1 << MAX_BITS_INDEX) -1,
        MAX_MAGIC = (1 << MAX_BITS_MAGIC) -1,
    };

    union {
        struct {
            u16 index;
            u16 magic;
        };
        u32 handle;
    };

    Handle() : handle(0) {}

    void Initialize(u32 index);

    inline bool operator==(const Handle &other) { return handle == other.handle; }
    inline bool operator!=(const Handle &other) { return handle != other.handle; }

    inline bool IsNull() { return !handle; }
    inline u32 GetIndex() { return (u32)index; }
    inline u32 GetMagic() { return (u32)magic; }
};

template <typename T>
struct AssetManager {
    
    AssetManager() = default;
    virtual ~AssetManager() = default;

    void Initialize(u32 size);
    void Terminate();
    
    Array<T>   userVec;
    Array<u32> magicVec;
    Array<u32> freeVec;

    HashMap<Handle> map;

    Handle GetAsset(const char *name);
    void   DeleteAsset(Handle handle);
    void   ClearAssets();
    
    virtual void Load(T *data, const char *name) {};
    virtual void Unload(T *data) {};

    T *Acquire(Handle &handle);
    void Release(Handle handle);
    T *Dereference(Handle handle);
    bool IsInFreeList(u32 index);
};

template <typename T>
void AssetManager<T>::Initialize(u32 size) {
    userVec.Initialize(size);
    magicVec.Initialize(size);
    freeVec.Initialize(size);
    map.Initialize(size*2);
}

template <typename T>
void AssetManager<T>::Terminate() {
    ClearAssets();
}


template <typename T>
Handle AssetManager<T>::GetAsset(const char *name) {
    
    Handle *handlePtr = map.Get(name);
    if(handlePtr) return *handlePtr;

    Handle handle;
     
    T* data = Acquire(handle);
    map.Add(name, handle);
    Load(data, name);
    
    return handle;
}

template <typename T>
void AssetManager<T>::DeleteAsset(Handle handle) {
    if(index < userVec.size && magicVec[index] == handle.GetMagic()) {
        T *data = Dereference(handle);
        Unload(data);
        Release(handle);
    } else {
        ASSERT(!"Invalid handle");
    }
}

template <typename T>
bool AssetManager<T>::IsInFreeList(u32 index) {
    for(u32 i = 0; i < freeVec.size; ++i) {
        if(index == freeVec[i]) return true;
    }
    return false;
}

template <typename T>
void AssetManager<T>::ClearAssets() {
    
    for(u32 index = 0; index < userVec.size; ++index)   {
        if(IsInFreeList(index)) continue;
        Unload(&userVec[index]);
    }
    
    userVec.size  = 0;
    magicVec.size = 0;
    freeVec.size  = 0;

    map.Clear();
}

template <typename T>
T *AssetManager<T>::Acquire(Handle &handle) {
    
    u32 index = 0;

    if(freeVec.size == 0) {
        index = magicVec.size;
        handle.Initialize(index);
        userVec.Push(T());
        magicVec.Push(handle.GetMagic());
    } else {
        index = freeVec[freeVec.size - 1];
        handle.Initialize(index);
        freeVec.size -= 1;
        magicVec[index] = handle.GetMagic();
    }
    
    return &userVec[index];
}

template <typename T>
void AssetManager<T>::Release(Handle handle) {
    u32 index = handle.GetIndex();
    ASSERT(index < userVec.size);
    ASSERT(magicVec[index] == handle.GetMagic());
    
    magicVec[index] = 0;
    freeVec.Push(index);
}

template <typename T>
T *AssetManager<T>::Dereference(Handle handle) {
    if(handle.IsNull()) return nullptr;

    u32 index = handle.GetIndex();
    ASSERT(index < userVec.size);
    ASSERT(magicVec[index] == handle.GetMagic());

    return &userVec[index];
}


#endif // _ASSET_MANAGER_H_
