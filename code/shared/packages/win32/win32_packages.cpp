#include <packages/packages.cpp>
#include <os_shared/win32/win32_shared.cpp>

struct win32_file_resource : resource {
    HANDLE                 Handle;
};

struct win32_packages : packages {
    HANDLE IOCompletionPort;
    fixed_array<ak_thread> Threads;
};

struct win32_io_read_task {
    OVERLAPPED                         Overlapped;
    void*                              Buffer;
    package_io_task                    Task;
    ak_auto_reset_event                Event;
    resource_load_async_callback_func* Callback;
    void*                              UserData;
};

static_assert(offsetof(win32_io_read_task, Overlapped) == 0);

internal void Win32_File_System_Build_Package_Resources(win32_packages* Packages, domain* Domain, package* Package, wstring DomainPathW, wstring PackageNameW) {
    scratch Scratch = Scratch_Get();
    wstring PackagePathW = WString_Concat(&Scratch, {DomainPathW, WString_Lit("\\"), PackageNameW});
    wstring WildcardPathW = WString_Concat(&Scratch, PackagePathW, WString_Lit("\\*"));

    WIN32_FIND_DATAW FindData;
    HANDLE File = FindFirstFileW(WildcardPathW.Str, &FindData);
    while(File != INVALID_HANDLE_VALUE) {
        wstring FileW = wstring(FindData.cFileName);
        if(FileW != WString_Lit(".") && FileW != WString_Lit("..")) {          
            wstring FilePathW = WString_Concat(&Scratch, {PackagePathW, WString_Lit("\\"), FileW});
            if(Win32_File_Exists(FilePathW.Str)) {
                HANDLE ResourceHandle = CreateFileW(FilePathW.Str, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY|FILE_FLAG_OVERLAPPED, NULL);
                if(ResourceHandle != INVALID_HANDLE_VALUE) {
                    if(CreateIoCompletionPort(ResourceHandle, Packages->IOCompletionPort, 0, 0) != NULL) {
                        wstring ResourceNameW = WString_Get_Filename_Without_Ext(FilePathW);
                        win32_file_resource* Resource = Arena_Push_Struct(Packages->Arena, win32_file_resource);
                        Resource->Name = string(Packages->AsyncAllocator, ResourceNameW);
                        Resource->Package = Package;
                        DLL_Push_Back(Packages->ResourceListHead, Packages->ResourceListTail, Resource);
                        DLL_Push_Back_NP(Package->FirstResource, Package->LastResource, Resource, NextInPackageList, PrevInPackageList);

                        Packages->ResourceCount++;
                        Package->ResourceCount++;

                        Resource->Hash = Hash_CRC(Resource->Name.Str, Resource->Name.Size, Package->Hash);
                        Resource->Handle = ResourceHandle;
                        Hashmap_Add_By_Hash(&Packages->ResourceMap, Resource->Name, Resource->Hash, (resource*)Resource);
                    } else {
                        Assert(false);
                        CloseHandle(ResourceHandle);
                    }
                }
            }
        }

        if(!FindNextFileW(File, &FindData)) {
            break;
        }
    }
}

internal void Win32_File_System_Build_Domain_Packages(win32_packages* Packages, domain* Domain, wstring RootPathW, wstring DomainNameW) {
    scratch Scratch = Scratch_Get();
    wstring DomainPathW = WString_Concat(&Scratch, {RootPathW, DomainNameW});
    wstring WildcardPathW = WString_Concat(&Scratch, DomainPathW, WString_Lit("\\*"));

    WIN32_FIND_DATAW FindData;
    HANDLE File = FindFirstFileW(WildcardPathW.Str, &FindData);
    while(File != INVALID_HANDLE_VALUE) {
        wstring FileW = wstring(FindData.cFileName);
        if(FileW != WString_Lit(".") && FileW != WString_Lit("..")) {          
            wstring FilePathW = WString_Concat(&Scratch, {DomainPathW, WString_Lit("\\"), FileW});
            if(Win32_Directory_Exists(FilePathW.Str)) {
                wstring PackageNameW = WString_Get_Filename_Without_Ext(FilePathW);
                package* Package = Arena_Push_Struct(Packages->Arena, package);
                Package->Name = string(Packages->AsyncAllocator, PackageNameW);
                Package->Domain = Domain;
                DLL_Push_Back(Packages->PackageListHead, Packages->PackageListTail, Package);
                DLL_Push_Back_NP(Domain->FirstPackage, Domain->LastPackage, Package, NextInDomainList, PrevInDomainList);
                Packages->PackageCount++;
                Domain->PackageCount++;

                Package->Hash = Hash_CRC(Package->Name.Str, Package->Name.Size, Domain->Hash);
                Hashmap_Add_By_Hash(&Packages->PackageMap, Package->Name, Package->Hash, Package);
                Win32_File_System_Build_Package_Resources(Packages, Domain, Package, DomainPathW, PackageNameW);
            }
        }

        if(!FindNextFileW(File, &FindData)) {
            break;
        }
    }
}

internal AK_THREAD_CALLBACK_DEFINE(Win32_IO_Thread_Callback) {
    win32_packages* Packages = (win32_packages*)UserData;

    DWORD       BytesCopied   = 0;
    OVERLAPPED* Overlapped    = NULL;
    ULONG_PTR   CompletionKey = 0;

    while(GetQueuedCompletionStatus(Packages->IOCompletionPort, &BytesCopied, &CompletionKey, &Overlapped, INFINITE)) {
        if(BytesCopied == 0 && CompletionKey == 0 && Overlapped == NULL) {
            break;
        } else {
            Assert(Overlapped);
            win32_io_read_task* ReadTask = (win32_io_read_task*)Overlapped;
            const_buffer Buffer(ReadTask->Buffer, BytesCopied);
            ReadTask->Callback(Buffer, ReadTask->UserData);
            AK_Auto_Reset_Event_Signal(&ReadTask->Event);
            AK_Auto_Reset_Event_Delete(&ReadTask->Event);
            Packages_Delete_IO_Task(Packages, ReadTask->Task);
        }
    }
    return 0;
}

packages* Packages_Create(const packages_create_info& CreateInfo) {
    arena* PackageArena = Arena_Create(Core_Get_Base_Allocator());
    win32_packages* Result = Arena_Push_Struct(PackageArena, win32_packages);
    Result->Arena = PackageArena;
    Packages_Init(Result, CreateInfo);

    Hashmap_Init(&Result->DomainMap, Result->AsyncAllocator);
    Hashmap_Init(&Result->PackageMap, Result->AsyncAllocator);
    Hashmap_Init(&Result->ResourceMap, Result->AsyncAllocator);

    u32* IOTaskIndices = Arena_Push_Array(Result->Arena, CreateInfo.IOTaskCount, u32);
    ak_slot64* IOTaskSlots = Arena_Push_Array(Result->Arena, CreateInfo.IOTaskCount, ak_slot64);
    void** IOTasks = Arena_Push_Array(Result->Arena, CreateInfo.IOTaskCount, void*);

    AK_Async_Slot_Map64_Init_Raw(&Result->IOTaskSlots, IOTaskIndices, IOTaskSlots, CreateInfo.IOTaskCount);
    Result->IOTasks = IOTasks;

    Result->IOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, CreateInfo.NumIOThreads);
    if(!Result->IOCompletionPort) { return NULL; }

    Array_Init(&Result->Threads, Result->Arena, CreateInfo.NumIOThreads);
    for(ak_thread& Thread : Result->Threads) {
        AK_Thread_Create(&Thread, Win32_IO_Thread_Callback, Result);
    }

    scratch Scratch = Scratch_Get();
    wstring RootPathW(&Scratch, CreateInfo.RootPath);

    switch(Result->Type) {
        case packages_type::FileSystem: {
            wstring WildcardPathW = WString_Concat(&Scratch, RootPathW, WString_Lit("*"));
            WIN32_FIND_DATAW FindData;
            HANDLE File = FindFirstFileW(WildcardPathW.Str, &FindData);
            while(File != INVALID_HANDLE_VALUE) {
                wstring FileW = wstring(FindData.cFileName);
                if(FileW != WString_Lit(".") && FileW != WString_Lit("..")) {          
                    wstring FilePathW = WString_Concat(&Scratch, {RootPathW, FileW});
                    if(Win32_Directory_Exists(FilePathW.Str)) {
                        domain* Domain = Arena_Push_Struct(Result->Arena, domain);
                        wstring DomainNameW = WString_Get_Filename_Without_Ext(FilePathW);
                        Domain->Name = string(Result->AsyncAllocator, DomainNameW);
                        DLL_Push_Back(Result->DomainListHead, Result->DomainListTail, Domain);
                        Result->DomainCount++;

                        Domain->Hash = Hash_CRC(Domain->Name.Str, Domain->Name.Size, 0);
                        Hashmap_Add_By_Hash(&Result->DomainMap, Domain->Name, Domain->Hash, Domain);

                        Win32_File_System_Build_Domain_Packages(Result, Domain, RootPathW, DomainNameW);
                    }
                }

                if(!FindNextFileW(File, &FindData)) {
                    break;
                }
            }
        } break;

        Invalid_Default_Case();
    }

    return Result;
}

internal package_io_task Win32_Read_Resource_Async(packages* Packages, resource* Resource, HANDLE FileHandle, u64 Offset, u32 Size, resource_load_async_callback_func* Callback, void* UserData) {
    package_io_task Task = Packages_Create_IO_Task(Packages, sizeof(win32_io_read_task)+Size);
    win32_io_read_task* ReadTask = (win32_io_read_task*)Packages_Get_IO_Task(Packages, Task);
    if(!ReadTask) return 0;    

    ReadTask->Buffer = (void*)(ReadTask+1);
    ReadTask->Task   = Task;
    ReadTask->Callback = Callback;
    ReadTask->UserData = UserData;
    AK_Auto_Reset_Event_Create(&ReadTask->Event, 0);

    //OVERLAPPED should always be the first member of all completion tasks
    OVERLAPPED* Overlapped = &ReadTask->Overlapped;
    Overlapped->Offset = (u32)Offset;
    Overlapped->OffsetHigh = (u32)(Offset >> 32); 

    if(!ReadFile(FileHandle, ReadTask->Buffer, Size, NULL, Overlapped)) {
        DWORD ErrorCode = GetLastError();
        if(ErrorCode != ERROR_IO_PENDING) {
            Packages_Delete_IO_Task(Packages, Task);
            return {};
        }
    }

    return Task;
}

internal RESOURCE_LOAD_ASYNC_CALLBACK_DEFINE(Package_Fill_Buffer) {
    buffer* Buffer = (buffer*)UserData;
    Assert(Buffer->Size == ResourceBuffer.Size);
    Memory_Copy(Buffer->Ptr, ResourceBuffer.Ptr, Buffer->Size);
}

buffer Packages_Load_Entire_Resource(packages* Packages, resource* Resource_, allocator* Allocator) {
    switch(Packages->Type) {
        case packages_type::FileSystem: {
            win32_file_resource* Resource = (win32_file_resource*)Resource_;
            
            LARGE_INTEGER FileSizeLI;
            GetFileSizeEx(Resource->Handle, &FileSizeLI);
            u32 FileSize = Safe_U32((u64)FileSizeLI.QuadPart);

            buffer Buffer(Allocator, FileSize);
            package_io_task Task = Win32_Read_Resource_Async(Packages, Resource, Resource->Handle, 0, FileSize, Package_Fill_Buffer, &Buffer);
            Packages_Wait_On_Task(Packages, Task);
            return Buffer;
        } break;

        Invalid_Default_Case();
    }

    return {};
}

package_io_task Packages_Load_Entire_Resource_Async(packages* Packages, resource* Resource_, resource_load_async_callback_func* ResourceLoadFunc, void* UserData) {
    switch(Packages->Type) {
        case packages_type::FileSystem: {
            win32_file_resource* Resource = (win32_file_resource*)Resource_;
            
            LARGE_INTEGER FileSizeLI;
            GetFileSizeEx(Resource->Handle, &FileSizeLI);
            u32 FileSize = Safe_U32((u64)FileSizeLI.QuadPart);

            package_io_task Task = Win32_Read_Resource_Async(Packages, Resource, Resource->Handle, 0, FileSize, ResourceLoadFunc, UserData);
            return Task;
        } break;

        Invalid_Default_Case();
    }

    return {};
}

void Packages_Wait_On_Task(packages* Packages, package_io_task Task) {
    win32_io_read_task* ReadTask = (win32_io_read_task*)Packages_Get_IO_Task(Packages, Task);
    if(ReadTask) {
        AK_Auto_Reset_Event_Wait(&ReadTask->Event);
    }
}
