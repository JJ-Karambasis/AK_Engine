#include "gl_gpu.h"

void GL_Debug_Message_Callback(GLenum Source, GLenum Type, GLuint ID, GLenum Severity, GLsizei Length, const GLchar* Message,
                               const void* UserData)
{
    //Assert(Severity != GL_DEBUG_SEVERITY_MEDIUM || Severity != GL_DEBUG_SEVERITY_HIGH);
}

shared_export GPU_INIT(GPU_Init)
{
    Core_Set(Core);
    
    arena* Arena = Arena_Create(OS_Get_Allocator(), Mega(4));
    gl* GL = Arena_Push_Struct(Arena, gl);
    Zero_Struct(GL, gl);
    
    GL->Arena = Arena;
    GL->ContextManager = GL_Context_Manager_Create(Get_Base_Allocator(Arena));
    
    gl_device* Device = Arena_Push_Struct(Arena, gl_device);
    
    Device->Context = GL_Context_Manager_Create_Default(GL->ContextManager);
    GL_Context_Make_Current(Device->Context);
    Device->Device.DeviceName = Str8_Copy(Get_Base_Allocator(Arena), Str8_Null_Term(glGetString(GL_RENDERER)));
    GL_Context_Make_Current(NULL);
    
    GL->Context.DeviceList.Count = 1;
    GL->Context.DeviceList.Devices = (gpu_device**)(&Device);
    
    return &GL->Context;
}

shared_export GPU_SHUTDOWN(GPU_Shutdown)
{
}

shared_export GPU_RELOAD(GPU_Reload)
{
    //TODO(JJ): Implement at some point
}

#include "Private/gl_context.c"
#include "Private/gl_device.c"
#include "Private/gl_resource.c"
#include "Private/gl_cmd_buffer.c"
#include "Private/gl_shader.c"
#include <Core/core.c>