#ifndef GL_GPU_H
#define GL_GPU_H

typedef struct gl gl;

#include <Core/core.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include "../gpu.h"
#include "Public/gl_device.h"
#include "Public/gl_context.h"
#include "Public/gl_resource.h"
#include "Public/gl_cmd_buffer.h"
#include "Public/gl_shader.h"

typedef struct gl
{
    gpu_context         Context;
    arena*              Arena;
    gl_context_manager* ContextManager;
} gl;

#endif