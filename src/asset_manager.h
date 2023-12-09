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
    inline u32 GeIndex() { return (u32)index; }
    inline u32 GeMagic() { return (u32)magic; }
};

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

template <typename T, u32 Size>
struct AssetManager {

    virtual ~AssetManager() = default;
    
    StaticArray<T, Size>   userVec;
    StaticArray<u32, Size> magicVec;
    StaticArray<u32, Size> freeVec;

    StaticHashMap<Handle, Size*2> map;
    
    Handle GetAsset(const char *name);
    void DeleteAsset(Handle handle);
    void ClearAssets(); 
    
    virtual void Load(T *data)   {};
    virtual void Unload(T *data) {};

    // NOTE: Singleton
    inline AssetManager *Get() { return assetManager; }
    static AssetManager assetManager;

protected:
    
    T *Acquire(Handle &handle);
    void Release(Handle handle);
    T *Dereference(Handle handle);
    bool IsInFreeList(u32 index);

};

template <typename T, u32 Size>
Handle AssetManager<T, Size>::GetAsset(const char *name) {
    
    Handle *handlePtr = map.Get(name);
    if(handlePtr) return *handlePtr;

    Handle handle;;
     
    T* data = Acquire(*handle);
    map.Add(name, *handle);
    Load(data);
    
    return handle;
}

template <typename T, u32 Size>
void AssetManager<T, Size>::DeleteAsset(Handle handle) {
    if(index < userVec.size && magicVec[index] == handle.GetMagic()) {
        T *data = Dereference(handle);
        Unload(data);
        Release(handle);
    } else {
        ASSERT(!"Invalid handle");
    }
}

template <typename T, u32 Size>
bool AssetManager<T, Size>::IsInFreeList(u32 index) {
    for(u32 i = 0; i < freeVec.size; ++i) {
        if(index == freeVec[i]) return true;
    }
    return false;
}

template <typename T, u32 Size>
void AssetManager<T, Size>::ClearAssets() {
    
    for(u32 index = 0; index < userVec.size; ++index)   {
        if(IsInFreeList(index)) continue;
        Unload(&userVec[index]);
    }
    
    userVec.size  = 0;
    magicVec.size = 0;
    freeVec.size  = 0;

    map.Clear();
}

template <typename T, u32 Size>
T *AssetManager<T, Size>::Acquire(Handle &handle) {
    
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
        magicVec[index] = handle.GeMagic();
    }
    
    return &userVec[index];
}

template <typename T, u32 Size>
void AssetManager<T, Size>::Release(Handle handle) {
    u32 index = handle.GetIndex();
    ASSERT(index < userVec.size);
    ASSERT(magicVec[index] == handle.GetMagic());
    
    magicVec[index] = 0;
    freeVec.Push(index);
}

template <typename T, u32 Size>
T *AssetManager<T, Size>::Dereference(Handle handle) {
    if(handle.IsNull()) return nullptr;

    ASSERT(index < userVec.size);
    ASSERT(magicVec[index] == handle.GetMagic());

    return &userVec[index];
}

#endif // _ASSET_MANAGER_H_
