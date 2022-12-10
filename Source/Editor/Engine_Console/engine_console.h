#ifndef ENGINE_CONSOLE_H
#define ENGINE_CONSOLE_H

typedef struct engine_console 
{
    arena* Arena;
} engine_console;

engine_console* Engine_Console_Create(allocator* Allocator);
void            Engine_Console_Delete(engine_console* Console);

#endif