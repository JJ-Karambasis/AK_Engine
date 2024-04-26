UTEST(Atlas_Allocator, Basic) {
    scratch Scratch = Scratch_Get();
    atlas_allocator* Allocator = Atlas_Allocator_Create({
        .Allocator = &Scratch,
        .Dim = uvec2(1000, 1000)
    });

    atlas_alloc_id Full = Atlas_Allocator_Alloc(Allocator, uvec2(1000, 1000));
    ASSERT_FALSE(Atlas_Alloc_Is_Null(Full));

    atlas_alloc_id Failed = Atlas_Allocator_Alloc(Allocator, uvec2(1, 1));
    ASSERT_TRUE(Atlas_Alloc_Is_Null(Failed));

    Atlas_Allocator_Free(Allocator, Full);

    atlas_alloc_id A = Atlas_Allocator_Alloc(Allocator, uvec2(100, 1000));
    atlas_alloc_id B = Atlas_Allocator_Alloc(Allocator, uvec2(900, 200));
    atlas_alloc_id C = Atlas_Allocator_Alloc(Allocator, uvec2(300, 200));
    atlas_alloc_id D = Atlas_Allocator_Alloc(Allocator, uvec2(200, 300));
    atlas_alloc_id E = Atlas_Allocator_Alloc(Allocator, uvec2(100, 300));
    atlas_alloc_id F = Atlas_Allocator_Alloc(Allocator, uvec2(100, 300));
    atlas_alloc_id G = Atlas_Allocator_Alloc(Allocator, uvec2(100, 300));

    ASSERT_FALSE(Atlas_Alloc_Is_Null(A));
    ASSERT_FALSE(Atlas_Alloc_Is_Null(B));
    ASSERT_FALSE(Atlas_Alloc_Is_Null(C));
    ASSERT_FALSE(Atlas_Alloc_Is_Null(D));
    ASSERT_FALSE(Atlas_Alloc_Is_Null(E));
    ASSERT_FALSE(Atlas_Alloc_Is_Null(F));
    ASSERT_FALSE(Atlas_Alloc_Is_Null(G));

    Atlas_Allocator_Free(Allocator, B);
    Atlas_Allocator_Free(Allocator, F);
    Atlas_Allocator_Free(Allocator, C);
    Atlas_Allocator_Free(Allocator, E);

    atlas_alloc_id H = Atlas_Allocator_Alloc(Allocator, uvec2(500, 200));
    ASSERT_FALSE(Atlas_Alloc_Is_Null(H));

    Atlas_Allocator_Free(Allocator, A);

    atlas_alloc_id I = Atlas_Allocator_Alloc(Allocator, uvec2(500, 200));
    ASSERT_FALSE(Atlas_Alloc_Is_Null(I));

    Atlas_Allocator_Free(Allocator, G);
    Atlas_Allocator_Free(Allocator, H);
    Atlas_Allocator_Free(Allocator, D);
    Atlas_Allocator_Free(Allocator, I);

    Full = Atlas_Allocator_Alloc(Allocator, uvec2(1000, 1000));
    ASSERT_FALSE(Atlas_Alloc_Is_Null(Full));

    Failed = Atlas_Allocator_Alloc(Allocator, uvec2(1, 1));
    ASSERT_TRUE(Atlas_Alloc_Is_Null(Failed));

    Atlas_Allocator_Free(Allocator, Full);
}

UTEST(Atlas_Allocator, Random) {
    const u32 Iterations = 500000;

    scratch Scratch = Scratch_Get();
    atlas_allocator* Allocator = Atlas_Allocator_Create({
        .Allocator = &Scratch,
        .Dim       = uvec2(1000, 1000),
        .Alignment = uvec2(5, 2)
    });

    u32 Seed;
    Hash_Random(&Seed, sizeof(u32));

    srand(Seed);

    uptr Allocated = 0;
    uptr Misses = 0;

    array<atlas_alloc_id> AllocatedIDs(&Scratch);

    for(u32 i = 0; i < Iterations; i++) {
        if(rand() % 5 > 2 && !Array_Empty(&AllocatedIDs)) {
            uptr Index = rand() % AllocatedIDs.Count;
            atlas_alloc_id ID = AllocatedIDs[Index];
            Array_Remove(&AllocatedIDs, Index);
            Atlas_Allocator_Free(Allocator, ID);
        } else {
            uvec2 Size = uvec2((u32)(rand() % 300)+5, (u32)(rand() % 300) + 5);
            atlas_alloc_id ID = Atlas_Allocator_Alloc(Allocator, Size);

            if(!Atlas_Alloc_Is_Null(ID)) {
                Array_Push(&AllocatedIDs, ID);
                Allocated++;
            } else {
                Misses++;
            }
        }
    }

    while(AllocatedIDs.Count) {
        atlas_alloc_id AllocID = Array_Pop(&AllocatedIDs);
        Atlas_Allocator_Free(Allocator, AllocID);
    }

    printf("added/removed %llu rectangles, %llu misses\n", Allocated, Misses);

    atlas_alloc_id Full = Atlas_Allocator_Alloc(Allocator, uvec2(1000, 1000));
    ASSERT_FALSE(Atlas_Alloc_Is_Null(Full));

    atlas_alloc_id Failed = Atlas_Allocator_Alloc(Allocator, uvec2(1, 1));
    ASSERT_TRUE(Atlas_Alloc_Is_Null(Failed));
}