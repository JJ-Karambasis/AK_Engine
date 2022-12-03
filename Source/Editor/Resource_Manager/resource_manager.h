#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

typedef enum resource_type
{
    RESOURCE_FONT,
    RESOURCE_LANGUAGE,
    RESOURCE_COUNT
} resource_type;

typedef struct resource
{
    str8             Name;
    str8             Path;
    buffer           Data;
    struct resource* Next;
} resource;

typedef struct resource_manager
{
    arena*    Arena;
    resource* Resources[RESOURCE_COUNT];
} resource_manager;

resource_manager* Resource_Manager_Create(allocator* Allocator, str8 RootPath);
resource*         Resource_Manager_Get(resource_manager* Manager, resource_type Type, str8 Name);
buffer            Resource_Manager_Load(resource_manager* Manager, allocator* Allocator, resource_type Type, str8 Name);
void              Resource_Manager_Delete(resource_manager* ResourceManager);

#endif