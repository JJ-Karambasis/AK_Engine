#define AK_FBX_MALLOC(size, user_data) Scratch_Push((scratch*)user_data, (uptr)size)
#define AK_FBX_FREE(ptr, user_data) //noop
#define AK_FBX_IMPLEMENTATION
#include <ak_fbx.h>

static string AK_FBX_To_String(allocator* Allocator, ak_fbx_string String) {
    return string(Allocator, String.Str, (uptr)String.Size);
}

fbx_scene* FBX_Create_Scene(allocator* Allocator, string Path) {
    scratch Scratch = Scratch_Get();
    buffer Buffer = OS_Read_Entire_File(&Scratch, Path);
    if(Buffer.Is_Empty()) {
        //todo: Logging
        return NULL;
    }

    ak_fbx_scene* SrcScene = AK_FBX_Load_From_Memory(Buffer.Ptr, Buffer.Size, &Scratch);
    if(!SrcScene) {
        //todo: Logging
        return NULL;
    }

    arena* Arena = Arena_Create(Allocator);
    if(!Arena) {
        //todo: Logging
        return NULL;
    }

    fbx_scene* DstScene = Arena_Push_Struct(Arena, fbx_scene);
    DstScene->Arena = Arena;

    DstScene->Meshes = array<fbx_mesh>(DstScene->Arena, SrcScene->Geometries.Count);
    DstScene->Objects = array<fbx_object>(DstScene->Arena, SrcScene->Nodes.Count);

    for(u64 i = 0; i < SrcScene->Nodes.Count; i++) {
        ak_fbx_node* Node = SrcScene->Nodes.Ptr[i];
        switch(Node->Type) {
            case AK_FBX_NODE_TYPE_GEOMETRY: {
                fbx_object Object = {};
                Object.Name      = AK_FBX_To_String(DstScene->Arena, Node->Name);
                Matrix4_Affine_Init(&Object.Transform, Node->GlobalTransform.Data);
                ak_fbx_material_array Materials = AK_FBX_Node_Get_Materials(Node);
                Array_Init(&Object.MeshIndices, DstScene->Arena, Materials.Count);
                ak_fbx_geometry* Geometry = AK_FBX_Node_Get_Geometry(Node);
                ak_fbx_polygons* Polygons = &Geometry->Polygons;
                ak_fbx_s32_array* MaterialIndices = &Polygons->MaterialIndices;
                ak_fbx_uv_map* UVDataMap = Geometry->UVMaps.Count ? &Geometry->UVMaps.Ptr[0] : NULL;

                hashmap<u32, u32> MaterialMeshMap(&Scratch, Safe_U32(Materials.Count));
                
                array<array<vec3>> MeshPositions(&Scratch, Materials.Count);
                array<array<u32>> MeshPositionVtxIndices(&Scratch, Materials.Count);
                array<hashmap<u32, u32>> MeshPositionMap(&Scratch, Materials.Count);

                array<array<vec2>> MeshUVs(&Scratch, Materials.Count);
                array<array<u32>> MeshUVVtxIndices(&Scratch, Materials.Count);
                array<hashmap<u32, u32>> MeshUVMap(&Scratch, Materials.Count);

                for(u32 j = 0; j < Safe_U32(Materials.Count); j++) {
                    Hashmap_Add(&MaterialMeshMap, j, Safe_U32(DstScene->Meshes.Count));
                    Array_Push(&Object.MeshIndices, DstScene->Meshes.Count);
                    
                    Array_Push(&MeshPositions, array<vec3>(&Scratch, Geometry->Vertices.Count));
                    Array_Push(&MeshPositionVtxIndices, array<u32>(&Scratch, Polygons->VertexIndices.Count));
                    Array_Push(&MeshPositionMap, hashmap<u32, u32>(&Scratch, Safe_U32(Polygons->VertexIndices.Count)));
                    
                    Array_Push(&MeshUVs, array<vec2>(&Scratch, Geometry->Vertices.Count));
                    Array_Push(&MeshUVVtxIndices, array<u32>(&Scratch, Polygons->VertexIndices.Count));
                    Array_Push(&MeshUVMap, hashmap<u32, u32>(&Scratch, Safe_U32(Polygons->VertexIndices.Count)));

                    Array_Push(&DstScene->Meshes, {});
                }

                u64 PolygonIndex = 0;
                while(PolygonIndex < Polygons->PolygonArray.Count) {
                    ak_fbx_polygon* Polygon = Polygons->PolygonArray.Ptr + PolygonIndex;                    
                    uptr MaterialIndex = (uptr)MaterialIndices->Ptr[Polygon->IndexArrayOffset];
                    
                    array<vec3>& Positions = MeshPositions[MaterialIndex];
                    array<u32>& PositionVtxIndices = MeshPositionVtxIndices[MaterialIndex];
                    hashmap<u32, u32>& PositionMap = MeshPositionMap[MaterialIndex];
                    
                    array<vec2>& UVs = MeshUVs[MaterialIndex];
                    array<u32>& UVVtxIndices = MeshUVVtxIndices[MaterialIndex];
                    hashmap<u32, u32>& UVMap = MeshUVMap[MaterialIndex];
                    
                    fbx_mesh* Mesh = &DstScene->Meshes[Object.MeshIndices[MaterialIndex]];
                    
                    while(PolygonIndex < Polygons->PolygonArray.Count) {
                        Polygon = Polygons->PolygonArray.Ptr + PolygonIndex;                    
                        u32 VtxIdx = Polygon->IndexArrayOffset;
                        if(MaterialIndices->Ptr[VtxIdx] != (s32)MaterialIndex) 
                            break;

                        u32 VtxCount = VtxIdx+Polygon->IndexArrayCount;
                        Assert(Polygon->IndexArrayCount == 3); //Must be a triangle mesh
                        for(; VtxIdx < VtxCount; VtxIdx++) {
                            u32 SrcVtxIdx = (u32)Polygons->VertexIndices.Ptr[VtxIdx];
                            if(Hashmap_Find(&PositionMap, SrcVtxIdx)) {
                                u32 Idx = Hashmap_Get_Value(&PositionMap);
                                Array_Push(&PositionVtxIndices, Idx);
                            } else {
                                u32 DstVtxIdx = Safe_U32(Positions.Count);
                                Hashmap_Add(&PositionMap, SrcVtxIdx, DstVtxIdx);
                                Array_Push(&PositionVtxIndices, DstVtxIdx);
                                Array_Push(&Positions, vec3(Geometry->Vertices.Ptr[SrcVtxIdx].Data));
                            }
                        }

                        if(UVDataMap) {
                            VtxIdx = Polygon->IndexArrayOffset;
                            for(; VtxIdx < VtxCount; VtxIdx++) {
                                u32 SrcVtxIdx = (u32)UVDataMap->UVIndices.Ptr[VtxIdx];
                                if(Hashmap_Find(&UVMap, SrcVtxIdx)) {
                                    u32 Idx = Hashmap_Get_Value(&UVMap);
                                    Array_Push(&UVVtxIndices, Idx);
                                } else {
                                    u32 DstVtxIdx = Safe_U32(UVs.Count);
                                    Hashmap_Add(&UVMap, SrcVtxIdx, DstVtxIdx);
                                    Array_Push(&UVVtxIndices, DstVtxIdx);
                                    Array_Push(&UVs, vec2(UVDataMap->UVs.Ptr[SrcVtxIdx].Data));
                                }
                            }
                        }

                        Mesh->TriangleCount++;
                        PolygonIndex++;
                    }
                }

                for(u64 j = 0; j < Materials.Count; j++) {
                    fbx_mesh* Mesh = &DstScene->Meshes[Object.MeshIndices[j]];
                    Mesh->Positions = fixed_array<vec3>(DstScene->Arena, MeshPositions[j]);
                    Mesh->PositionVtxIndices = fixed_array<u32>(DstScene->Arena, MeshPositionVtxIndices[j]);
                    Mesh->UVs = fixed_array<vec2>(DstScene->Arena, MeshUVs[j]);
                    Mesh->UVVtxIndices = fixed_array<u32>(DstScene->Arena, MeshUVVtxIndices[j]);
                }

                Array_Push(&DstScene->Objects, Object);
            } break;
        }
    }

    return DstScene;
}