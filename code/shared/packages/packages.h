#ifndef PACKAGES_H
#define PACKAGES_H

struct packages;
struct domain;
struct package;
struct resource;

#define RESOURCE_LOAD_ASYNC_CALLBACK_DEFINE(name) void name(const_buffer ResourceBuffer, void* UserData)
typedef RESOURCE_LOAD_ASYNC_CALLBACK_DEFINE(resource_load_async_callback_func);

enum class packages_type {
    FileSystem,
    Packed
};

struct packages_create_info {
    packages_type Type;
    string        RootPath;
    u32           NumIOThreads = AK_Get_Processor_Thread_Count();
    u32           IOTaskCount  = 1024;
};


typedef uint64_t package_io_task;

packages*       Packages_Create(const packages_create_info& CreateInfo);
domain*         Packages_Get_Domain(packages* Packages, string Domain);
package*        Packages_Get_Package(packages* Packages, domain* Domain, string Package);
package*        Packages_Get_Package(packages* Packages, string Domain, string Package);
resource*       Packages_Get_Resource(packages* Packages, string DomainName, string PackageName, string ResourceName);
resource*       Packages_Get_Resource(packages* Packages, domain* Domain, string PackageName, string ResourceName);
resource*       Packages_Get_Resource(packages* Packages, package* Package, string ResourceName);
buffer          Packages_Load_Entire_Resource(packages* Packages, resource* Resource, allocator* Allocator);
package_io_task Packages_Load_Entire_Resource_Async(packages* Packages, resource* Resource, resource_load_async_callback_func* ResourceLoadFunc, void* UserData);
void            Packages_Wait_On_Task(packages* Packages, package_io_task Task);

void            Packages_Delete(packages* Packages);

#endif