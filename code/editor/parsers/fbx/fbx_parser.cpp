#ifdef FBX_PARSER_USE_AK_FBX
#include "ak_fbx/ak_fbx_parser.cpp"
#else
#error "Not Implemented"
#endif

void FBX_Delete_Scene(fbx_scene* Scene) {
    arena* Arena = Scene->Arena;
    Zero_Struct(Scene);
    Arena_Delete(Arena);
}