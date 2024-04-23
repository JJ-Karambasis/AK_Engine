#include <core/core.h>
#include "packages.h"

struct domain {
    string  Name;
    u32     Hash;
    domain* Next;
    domain* Prev;

    u32      PackageCount;
    package* FirstPackage;
    package* LastPackage;
};

struct package {
    string    Name;
    u32       Hash;
    domain*   Domain;
    resource* FirstResource;
    resource* LastResource;
    u32       ResourceCount;

    package* Next;
    package* Prev;
    package* NextInDomainList;
    package* PrevInDomainList;
};

struct resource {
    string   Name;
    u32      Hash;
    package* Package;

    resource* Next;
    resource* Prev;
    resource* NextInPackageList;
    resource* PrevInPackageList;
};

#define MAX_TASK_COUNT 1024

struct package_base_task {
    void* UserData;
};

struct packages {
    packages_type Type;
    
    //Allocators
    arena*          Arena;
    heap*           ResourceHeap;
    lock_allocator* AsyncAllocator;

    //Package lists
    u32     DomainCount;
    domain* DomainListHead;
    domain* DomainListTail;
    domain* FreeDomains;

    u32      PackageCount;
    package* PackageListHead;
    package* PackageListTail;
    package* FreePackages;

    u32       ResourceCount;
    resource* ResourceListHead;
    resource* ResourceListTail;
    resource* FreeResources;

    //Maps
    hashmap<string, domain*>   DomainMap;
    hashmap<string, package*>  PackageMap;
    hashmap<string, resource*> ResourceMap; 

    ak_async_slot_map64 IOTaskSlots;
    void**              IOTasks;
};

void Packages_Init(packages* Packages, const packages_create_info& CreateInfo) {
    Packages->Type = CreateInfo.Type;
    Packages->ResourceHeap = Heap_Create(Core_Get_Base_Allocator());
    Packages->AsyncAllocator = Lock_Allocator_Create(Packages->ResourceHeap);
}

package_io_task Packages_Create_IO_Task(packages* Packages, uptr TaskSize) {
    ak_slot64 Slot = AK_Async_Slot_Map64_Alloc_Slot(&Packages->IOTaskSlots);
    if(!Slot) return 0;

    Packages->IOTasks[AK_Slot64_Index(Slot)] = Allocator_Allocate_Memory(Packages->AsyncAllocator, TaskSize);
    return (package_io_task)Slot; 
}

void Packages_Delete_IO_Task(packages* Packages, package_io_task Task) {
    ak_slot64 Slot = (ak_slot64)Task;
    if(AK_Async_Slot_Map64_Is_Allocated(&Packages->IOTaskSlots, Slot)) {
        Allocator_Free_Memory(Packages->AsyncAllocator, Packages->IOTasks[AK_Slot64_Index(Slot)]);
        Packages->IOTasks[AK_Slot64_Index(Slot)] = nullptr;
        AK_Async_Slot_Map64_Free_Slot(&Packages->IOTaskSlots, Slot);
    }
}

void* Packages_Get_IO_Task(packages* Packages, package_io_task Task) {
    ak_slot64 Slot = (ak_slot64)Task;
    if(!AK_Async_Slot_Map64_Is_Allocated(&Packages->IOTaskSlots, Slot)) {
        return nullptr;
    }
    return Packages->IOTasks[AK_Slot64_Index(Slot)];
}

domain* Packages_Get_Domain(packages* Packages, string Domain) {
    u32 DomainHash = Hash_CRC(Domain.Str, Domain.Size);
    domain** pDomain = Hashmap_Find_By_Hash(&Packages->DomainMap, Domain, DomainHash);
    if(!pDomain) return nullptr;
    return *pDomain;
}

package* Packages_Get_Package(packages* Packages, domain* Domain, string Package) {
    u32 PackageHash = Hash_CRC(Package.Str, Package.Size, Domain->Hash);
    package** pPackage = Hashmap_Find_By_Hash(&Packages->PackageMap, Package, PackageHash); 
    if(!pPackage) return nullptr;
    return *pPackage;
}

package* Packages_Get_Package(packages* Packages, string Domain, string Package) {
    u32 DomainHash = Hash_CRC(Domain.Str, Domain.Size);
    u32 PackageHash = Hash_CRC(Package.Str, Package.Size, DomainHash);
    package** pPackage = Hashmap_Find_By_Hash(&Packages->PackageMap, Package, PackageHash); 
    if(!pPackage) return nullptr;
    return *pPackage;
}

resource* Packages_Get_Resource(packages* Packages, domain* Domain, string PackageName, string ResourceName) {
    u32 PackageHash = Hash_CRC(PackageName.Str, PackageName.Size, Domain->Hash);
    u32 ResourceHash = Hash_CRC(ResourceName.Str, ResourceName.Size, PackageHash);
    resource** pResource = Hashmap_Find_By_Hash(&Packages->ResourceMap, ResourceName, ResourceHash); 
    if(!pResource) return nullptr;
    return *pResource;
}

resource* Packages_Get_Resource(packages* Packages, package* Package, string ResourceName) {
    u32 ResourceHash = Hash_CRC(ResourceName.Str, ResourceName.Size, Package->Hash);
    resource** pResource = Hashmap_Find_By_Hash(&Packages->ResourceMap, ResourceName, ResourceHash); 
    if(!pResource) return nullptr;
    return *pResource;
}

void      Packages_Delete(packages* Packages);