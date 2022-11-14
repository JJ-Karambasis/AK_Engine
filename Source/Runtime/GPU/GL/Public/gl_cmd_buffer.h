#ifndef GL_CMD_BUFFER_H
#define GL_CMD_BUFFER_H

typedef struct gl_cmd_storage
{
    arena*                 Arena;
    struct gl_cmd_storage* Next;
} gl_cmd_storage;

typedef struct gl_cmd_buffer
{
    gpu_cmd_buffer  CmdBuffer;
    gl_cmd_storage* CmdStorage;
} gl_cmd_buffer;

typedef struct gl_cmd_pool
{
    arena*          PoolMemory;
    gl_cmd_storage* FreeStorage;
} gl_cmd_pool;

typedef struct gl_cmd_upload_texture
{
    gpu_cmd       Cmd;
    gl_texture2D* DstTexture;
    uint32_t      DstOffsetX;
    uint32_t      DstOffsetY;
    void*         SrcTexels;
    uint32_t      SrcWidth;
    uint32_t      SrcHeight;
} gl_cmd_upload_texture;

gl_cmd_pool*   GL_Cmd_Pool_Create(allocator* Allocator);
void           GL_Cmd_Pool_Delete(gl_cmd_pool* CmdPool);
gl_cmd_buffer* GL_Cmd_Pool_Create_Buffer(gl_cmd_pool* CmdPool);
void           GL_Cmd_Pool_Delete_Buffer(gl_cmd_pool* CmdPool, gl_cmd_buffer* CmdBuffer);
GPU_CMD_UPLOAD_TEXTURE_PROC(GL_Cmd_Buffer_Upload_Texture);

#endif