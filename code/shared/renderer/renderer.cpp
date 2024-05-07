#define RENDER_TASK_INDEX_BIT_COUNT 30
#define RENDER_TASK_INVALID_INDEX (((u32)(1 << RENDER_TASK_INDEX_BIT_COUNT))-1)

//todo: Draw indirect and computer are not implemented yet
enum render_task_type {
    RENDER_TASK_TYPE_DRAW,
    RENDER_TASK_TYPE_DRAW_INDIRECT,
    RENDER_TASK_TYPE_COMPUTE,
    RENDER_TASK_TYPE_COUNT
};

union render_task_id_internal {
    render_task_id ID;
    struct {
        u32 Type       : 2;
        //Stores the current index if the entry is allocated
        //Stores the next free index if the entry is free
        u32 Index      : RENDER_TASK_INDEX_BIT_COUNT; 
        u32 Generation : 32;
    };
};

static_assert(sizeof(render_task_id_internal) == 8);

struct render_graph_entry;
struct render_graph_entry_list {
    render_graph_entry* First;
    render_graph_entry* Last;
};

struct render_texture_state {
    gdi_handle<gdi_texture> Attachment;
    gdi_resource_state      ResourceState;
    gdi_clear               Clear;
};

struct render_task {
    draw_stream                 DrawStream;
    draw_callback_func*         Callback;
    void*                       UserData;
    gdi_handle<gdi_render_pass> RenderPass;
    gdi_handle<gdi_framebuffer> Framebuffer;
    array<render_texture_state> RenderPassAttachmentStates;  
    u32                         DependentJobs;
    u32                         DependentIndex;
    gdi_cmd_list*               CmdList;
    render_graph_entry_list     Children;
    render_task*                Next;
};

struct render_task_pool {
    render_task*             Tasks;
    render_task_id_internal* IDs;
    u32                      FirstFreeIndex;
    u32                      MaxUsed;
    u32                      Capacity;
    render_task_type         Type;
};

struct render_graph_entry {
    render_task*        Task;
    render_graph_entry* NextSibling;
    render_graph_entry* PrevSibling;
};

struct render_graph {
    arena*                  Arena;
    renderer*               Renderer;
    render_graph_entry_list Children;
    render_task*            FirstTask;
    render_task*            LastTask;
    u64                     Generation;
    render_graph*           Next;
};

struct renderer {
    arena*           Arena;
    ak_job_system*   JobSystem;
    gdi_context*     Context;
    render_task_pool TaskPools[RENDER_TASK_TYPE_COUNT];
    render_graph*    FreeGraphs;
};

internal void Render_Task_Pool_Init(render_task_pool* Pool, render_task_type Type) {
    Zero_Struct(Pool);
    Pool->FirstFreeIndex = RENDER_TASK_INVALID_INDEX;
    Pool->Type = Type;
}

internal render_task_id Render_Task_Pool_Allocate(render_task_pool* Pool, arena* Arena) {
    u32 Index;

    //First check if the free list has an entry or not
    if(Pool->FirstFreeIndex != RENDER_TASK_INVALID_INDEX) {
        //If it does, grab that free index and pop it off
        Index = Pool->FirstFreeIndex;
        Pool->FirstFreeIndex = Pool->IDs[Index].Index;
    } else {
        //Check to make sure we have enough internal storage
        if(Pool->MaxUsed == Pool->Capacity) {
            //Allocate more storage if we ran out
            u32 NewCapacity = Pool->Capacity ? Pool->Capacity*2 : 32;
            uptr TotalAllocatedSize = (sizeof(render_task)+sizeof(render_task_id_internal))*NewCapacity;
            render_task* NewTasks = (render_task*)Arena_Push(Arena, TotalAllocatedSize);
            render_task_id_internal* NewIds = (render_task_id_internal*)(NewTasks+NewCapacity);
            
            for(uptr i = 0; i < NewCapacity; i++) {
                NewIds[i].Type = Pool->Type;
                NewIds[i].Generation = 1;
                NewIds[i].Index = RENDER_TASK_INVALID_INDEX;
            }

            if(Pool->Capacity > 0) {
                Array_Copy(NewTasks, Pool->Tasks, Pool->Capacity);
                Array_Copy(NewIds, Pool->IDs, Pool->Capacity);
            }

            Pool->Capacity = NewCapacity;
            Pool->Tasks = NewTasks;
            Pool->IDs = NewIds;
        }

        //Allocate a new index using MaxUsed
        Index = Pool->MaxUsed++;
    }

    Assert(Index < (u32)(1 << 31));
    Assert(Index < Pool->Capacity);
    render_task_id_internal* ID = Pool->IDs + Index;
    ID->Index = Index;
    return ID->ID;
}

internal void Render_Task_Pool_Free(render_task_pool* Pool, render_task_id _ID) {
    render_task_id_internal ID = {_ID};
    if(!ID.ID) return;

    Assert(ID.Type == (u32)Pool->Type);
    Assert(ID.Index < (u32)(1 << 31));
    Assert(ID.Index < Pool->Capacity);
    render_task_id_internal* PoolID = Pool->IDs + ID.Index;
    Assert(PoolID->Index == ID.Index);

    if(PoolID->Generation == ID.Generation) {
        PoolID->Generation++;
        
        //If the generation overflows, reset it back (old id handles may be valid now)
        if(!PoolID->Generation) {
            PoolID->Generation = 1;
        }

        PoolID->Index = Pool->FirstFreeIndex;
        Pool->FirstFreeIndex = ID.Index;
    }
}

internal render_task* Render_Task_Pool_Get(render_task_pool* Pool, render_task_id _ID) {
    render_task_id_internal ID = {_ID};
    if(!ID.ID) return nullptr;
    Assert(ID.Type == (u32)Pool->Type);
    Assert(ID.Index < (u32)(1 << 31));
    Assert(ID.Index < Pool->Capacity);
    render_task_id_internal* PoolID = Pool->IDs + ID.Index;
    Assert(PoolID->Index == ID.Index);
    return PoolID->Generation == ID.Generation ? (Pool->Tasks + ID.Index) : nullptr;
}

internal void Render_Task_Pool_Clear(render_task_pool* Pool) {
    for(u32 i = 0; i < Pool->Capacity; i++) {
        if(Pool->IDs[i].Index == i) {
            //Entry is allocated (indices match, invalidate handle)
            Pool->IDs[i].Generation++;
        }
        Pool->IDs[i].Index = RENDER_TASK_INVALID_INDEX;
    }

    Pool->MaxUsed = 0;
}

internal render_graph* Render_Graph_Get(render_graph_id GraphID) {
    if(!GraphID.ID || !GraphID.Graph) return nullptr;
    return GraphID.ID == GraphID.Graph->Generation ? GraphID.Graph : nullptr;
}

renderer* Renderer_Create(const renderer_create_info& CreateInfo) {
    arena* Arena = Arena_Create(CreateInfo.Allocator);
    renderer* Renderer = Arena_Push_Struct(Arena, renderer);
    Renderer->Arena = Arena;
    Renderer->JobSystem = CreateInfo.JobSystem;
    Renderer->Context = CreateInfo.Context;

    for(u32 i = 0; i < RENDER_TASK_TYPE_COUNT; i++) {
        Render_Task_Pool_Init(&Renderer->TaskPools[i], (render_task_type)i);
    }

    return Renderer;
}

void Renderer_Delete(renderer* Renderer) {
    if(Renderer && Renderer->Arena) {
        arena* Arena = Renderer->Arena;
        Arena_Delete(Arena);
    }
}

struct render_graph_draw_data {
    gdi_context* Context;
    render_task* Task;
};

internal AK_JOB_SYSTEM_CALLBACK_DEFINE(Render_Graph_Draw_Callback) {
    render_graph_draw_data* RenderGraphDrawData = (render_graph_draw_data*)JobUserData;
    render_task* RenderTask = RenderGraphDrawData->Task;
    gdi_context* Context = RenderGraphDrawData->Context;

    uvec2 Resolution = GDI_Context_Get_Framebuffer_Resolution(Context, RenderTask->Framebuffer);

    draw_stream* DrawStream = &RenderTask->DrawStream;
    RenderTask->Callback(JobSystem, Context, DrawStream, vec2(Resolution), RenderTask->UserData);

    scratch Scratch = Scratch_Get();

    fixed_array<gdi_clear> Clears(&Scratch, RenderTask->RenderPassAttachmentStates.Count);
    for(uptr i = 0; i < RenderTask->RenderPassAttachmentStates.Count; i++) {
        Clears[i] = RenderTask->RenderPassAttachmentStates[i].Clear;
    }

    gdi_cmd_list* CmdList = GDI_Context_Begin_Cmd_List(Context, GDI_CMD_LIST_TYPE_GRAPHICS, RenderTask->RenderPass, RenderTask->Framebuffer);

    RenderTask->CmdList = CmdList;
}

bool Renderer_Execute(renderer* Renderer, render_graph_id GraphID, span<gdi_handle<gdi_swapchain>> Swapchains) {
    render_graph* Graph = Render_Graph_Get(GraphID);
    Assert(Graph);

    ak_job_id RootJobID = AK_Job_System_Alloc_Empty_Job(Renderer->JobSystem, AK_JOB_FLAG_FREE_WHEN_DONE_BIT);

    scratch Scratch = Scratch_Get();
    hashmap<gdi_handle<gdi_texture>, gdi_resource_state> CurrentTextureStates(&Scratch);
    hashmap<gdi_handle<gdi_texture>, gdi_resource_state> InitialTextureStates(&Scratch);

    //Iterate through all the tasks and execute them. Ordering won't matter since
    //each task records into separate command lists 
    for(render_task* Task = Graph->FirstTask; Task; Task = Task->Next) {
        //Reset the dependency count
        Task->DependentIndex = 0;
        
        //Make sure a draw stream has data
        u8* DrawStream = (u8*)Scratch_Push(&Scratch, RENDERER_MAX_DRAW_STREAM_SIZE);
        Task->DrawStream = {
            .Start = DrawStream,
            .End = DrawStream + RENDERER_MAX_DRAW_STREAM_SIZE,
            .At = DrawStream
        };
        
        render_graph_draw_data DrawData = {
            .Context = Renderer->Context,
            .Task = Task
        };

        ak_job_system_data JobData = {
            .JobCallback = Render_Graph_Draw_Callback,
            .Data = &DrawData,
            .DataByteSize = sizeof(render_graph_draw_data)
        };

        for(render_texture_state State : Task->RenderPassAttachmentStates) {
            if(!Hashmap_Find(&CurrentTextureStates, State.Attachment)) {
                Hashmap_Add(&CurrentTextureStates, State.Attachment, GDI_RESOURCE_STATE_NONE);
            }
        }

        AK_Job_System_Alloc_Job(Renderer->JobSystem, JobData, RootJobID, AK_JOB_FLAG_FREE_WHEN_DONE_BIT|AK_JOB_FLAG_QUEUE_IMMEDIATELY_BIT);
    }

    AK_Job_System_Add_Job(Renderer->JobSystem, RootJobID);
    AK_Job_System_Wait_For_Job(Renderer->JobSystem, RootJobID);

    array<render_graph_entry_list> Stack(&Scratch);
    array<gdi_barrier> Barriers(&Scratch);
    array<gdi_swapchain_present_info> PresentInfo(&Scratch);

    gdi_cmd_list* MainCmdList = GDI_Context_Begin_Cmd_List(Renderer->Context, GDI_CMD_LIST_TYPE_GRAPHICS, {}, {});

    Array_Push(&Stack, Graph->Children);
    while(!Array_Empty(&Stack)) {
        array<render_graph_entry_list> NextLevels(&Scratch);
        while(!Array_Empty(&Stack)) {
            render_graph_entry_list List = Array_Pop(&Stack);
            for(render_graph_entry* Entry = List.First; Entry; Entry = Entry->NextSibling) {
                render_task* Task = Entry->Task;
                Task->DependentIndex++;
                if(Task->DependentIndex >= Task->DependentJobs) {
                    Assert(Task->DependentIndex == Task->DependentJobs);
                    span<render_texture_state> RenderPassAttachmentStates = Task->RenderPassAttachmentStates;
                    fixed_array<gdi_clear> Clears(&Scratch, RenderPassAttachmentStates.Count);
                    
                    for(uptr i = 0; i < RenderPassAttachmentStates.Count; i++) {
                        render_texture_state State = RenderPassAttachmentStates[i];
                        if(CurrentTextureStates[State.Attachment] != State.ResourceState) {
                            Array_Push(&Barriers, {
                                .Resource = gdi_resource::Texture(State.Attachment),
                                .OldState = CurrentTextureStates[State.Attachment],
                                .NewState = State.ResourceState
                            });
                            CurrentTextureStates[State.Attachment] = State.ResourceState;

                            if(!Hashmap_Find(&InitialTextureStates, State.Attachment)) {
                                Hashmap_Add(&InitialTextureStates, State.Attachment, State.ResourceState);
                            }
                        }
                        Clears[i] = RenderPassAttachmentStates[i].Clear;
                    }

                    if(Barriers.Count) {
                        GDI_Cmd_List_Barrier(MainCmdList, Barriers);
                        Array_Clear(&Barriers);
                    }

                    GDI_Cmd_List_Begin_Render_Pass(MainCmdList, {
                        .RenderPass = Task->RenderPass,
                        .Framebuffer = Task->Framebuffer, 
                        .ClearValues = Clears
                    });
                    GDI_Cmd_List_Execute_Cmds(MainCmdList, {Task->CmdList});
                    GDI_Cmd_List_End_Render_Pass(MainCmdList);
                }

            }
            Array_Push(&NextLevels, List);
        }

        for(render_graph_entry_list& NextLevel : NextLevels) {
            for(render_graph_entry* Entry = NextLevel.First; Entry; Entry = Entry->NextSibling) { 
                render_task* Task = Entry->Task;
                if(Task->Children.First && Task->Children.Last) {
                    Array_Push(&Stack, Task->Children);
                }
            }
        }
    }

    for(gdi_handle<gdi_swapchain> Swapchain : Swapchains) {
        span<gdi_handle<gdi_texture>> Textures = GDI_Context_Get_Swapchain_Textures(Renderer->Context, Swapchain);
        s32 TextureIndex = GDI_Context_Get_Swapchain_Texture_Index(Renderer->Context, Swapchain);
        Assert(TextureIndex != -1);
        gdi_handle<gdi_texture> Texture = Textures[(uptr)TextureIndex];
        if(CurrentTextureStates[Texture] != GDI_RESOURCE_STATE_PRESENT) {
            Array_Push(&Barriers, {
                .Resource = gdi_resource::Texture(Texture), 
                .OldState = CurrentTextureStates[Texture],
                .NewState = GDI_RESOURCE_STATE_PRESENT
            });
            Array_Push(&PresentInfo, {
                .Swapchain = Swapchain,
                .InitialState = InitialTextureStates[Texture]
            });
        }
    }

    if(Barriers.Count) {
        GDI_Cmd_List_Barrier(MainCmdList, Barriers);
        Array_Clear(&Barriers);
    }

    return GDI_Context_Execute(Renderer->Context, PresentInfo);    
}

render_task_id Render_Graph_Create_Draw_Task(renderer* Renderer, draw_callback_func* Callback, void* UserData) {
    render_task_pool* DrawPool = &Renderer->TaskPools[RENDER_TASK_TYPE_DRAW];

    render_task_id Result = Render_Task_Pool_Allocate(DrawPool, Renderer->Arena);
    if(!Result) return 0;

    render_task* RenderTask = Render_Task_Pool_Get(DrawPool, Result);

    RenderTask->Callback = Callback;
    RenderTask->UserData = UserData;
    return Result;
}

void Renderer_Delete_Task(renderer* Renderer, render_task_id TaskID) {
    //todo: Implement
    Not_Implemented();
}

render_graph_id Renderer_Create_Graph(renderer* Renderer) {
    render_graph* Graph = Renderer->FreeGraphs;
    if(Renderer->FreeGraphs) SLL_Pop_Front(Renderer->FreeGraphs);
    else {
        Graph = Arena_Push_Struct(Renderer->Arena, render_graph);
        Graph->Arena = Arena_Create(Renderer->Arena);
        Graph->Generation = 1;
        Graph->Renderer = Renderer;
    }

    return {
        .ID = Graph->Generation,
        .Graph = Graph
    };
}

void Renderer_Delete_Graph(renderer* Renderer, render_graph_id GraphID) {
    //todo: Implement
    Not_Implemented();
}

void Render_Graph_Add_Task(render_graph_id GraphID, render_task_id _TaskID, render_task_id _ParentID) {
    render_graph* Graph = Render_Graph_Get(GraphID);
    if(!Graph) {
        Assert(false);
        return;
    }
    renderer* Renderer = Graph->Renderer;
    
    render_task_id_internal TaskID = {_TaskID};
    render_task_id_internal ParentID = {_ParentID};

    if(!TaskID.ID || !ParentID.ID) {
        Assert(false);
        return;
    }
    
    render_task* RenderTask = Render_Task_Pool_Get(&Renderer->TaskPools[TaskID.Type], TaskID.ID);
    if(!RenderTask) {
        Assert(false);
        return;
    }

    render_graph_entry* GraphEntry = Arena_Push_Struct(Graph->Arena, render_graph_entry);
    GraphEntry->Task = RenderTask;
    
    RenderTask->DependentJobs++;

    render_task* ParentTask = Render_Task_Pool_Get(&Renderer->TaskPools[ParentID.Type], ParentID.ID);
    if(ParentTask) {
        DLL_Push_Back_NP(ParentTask->Children.First, ParentTask->Children.Last, GraphEntry, NextSibling, PrevSibling);
    } else {
        DLL_Push_Back_NP(Graph->Children.First, Graph->Children.Last, GraphEntry, NextSibling, PrevSibling);
    }

    if(!RenderTask->Next) {
        SLL_Push_Back(Graph->FirstTask, Graph->LastTask, RenderTask);
    }
}

void Render_Graph_Clear(render_graph_id GraphID) {
    render_graph* Graph = Render_Graph_Get(GraphID);
    Graph->Children = {};

    render_task* Task = Graph->FirstTask;
    while(Task) {
        render_task* TaskToRemove = Task;
        Task = Task->Next;
        
        TaskToRemove->Next = nullptr;
        TaskToRemove->Children = {};
    }

    Graph->FirstTask = nullptr;
    Graph->LastTask = nullptr;
    Arena_Clear(Graph->Arena);
}

#include "draw_stream.cpp"