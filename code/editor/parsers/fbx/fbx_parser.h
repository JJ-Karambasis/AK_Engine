#ifndef FBX_PARSER_H
#define FBX_PARSER_H

struct fbx_vtx {
    u32 PositionIndex;
    u32 NormalIndex;
    u32 UVIndex;
};

struct fbx_skeleton_vtx {
    u32 JointIndex;
    u32 WeightIndex;
};

struct fbx_mesh {
    fixed_array<vec3> Positions;
    fixed_array<vec3> Normals;
    fixed_array<vec2> UVs;
    u32               TriangleCount;
    fixed_array<u32>  PositionVtxIndices;
    fixed_array<u32>  NormalVtxIndices;
    fixed_array<u32>  UVVtxIndices;
    u32               SkeletonIndex = (u32)-1;
};

struct fbx_joint {
    string         Name;
    u32            ParentIndex;
    matrix4_affine LocalToJointTransform;
};

struct fbx_skeleton {
    u32              MeshIndex;
    array<fbx_joint> Joints;
    array<uvec4>     JointIndices;
    array<vec4>      JointWeights;
    u32*             JointVtxIndices;
    u32*             WeightVtxIndices;
};

struct fbx_animation {
    string Name;
    u32    ObjectIndex;
};

struct fbx_object {
    string         Name;
    matrix4_affine Transform;
    array<uptr>    MeshIndices;
};

struct fbx_scene {
    arena*               Arena;
    array<fbx_mesh>      Meshes;
    array<fbx_skeleton>  Skeletons;
    array<fbx_animation> Animations;
    array<fbx_object>    Objects;
};

fbx_scene* FBX_Create_Scene(allocator* Allocator, string Path);
void       FBX_Delete_Scene(fbx_scene* Scene);

#endif