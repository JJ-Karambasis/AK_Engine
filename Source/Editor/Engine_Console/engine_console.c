engine_console* Engine_Console_Create(allocator* Allocator)
{
    arena* Arena = Arena_Create(Allocator, Mega(1));
    engine_console* Result = Arena_Push_Struct(Arena, engine_console);
    Result->Arena = Arena;
    return Result;
}

void Engine_Console_Delete(engine_console* Console)
{
    Arena_Delete(Console->Arena);
}