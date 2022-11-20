uint32_t Glyph_Cache__Hash(font_face* Face, uint32_t Codepoint)
{
    uint32_t Result = Hash_Combine(Hash_Ptr((size_t)Face), Hash_U32(Codepoint));
    return Result;
}

glyph_cache* Glyph_Cache_Create(allocator* Allocator, glyph_generator* Generator, gpu_resource_manager* ResourceManager)
{
    arena* Arena = Arena_Create(Allocator, Mega(1));
    glyph_cache* Result = Arena_Push_Struct(Arena, glyph_cache);
    Zero_Struct(Result, glyph_cache);
    Result->Generator       = Generator;
    Result->ResourceManager = ResourceManager;
    Result->Arena           = Arena;
    Result->Slots           = Arena_Push_Array(Arena, glyph_slot, GLYPH_CACHE_SLOT_CAPACITY);
    
    return Result;
}

glyph* Glyph_Cache_Get(glyph_cache* Cache, font_face* Face, uint32_t Codepoint)
{
    uint32_t Hash = Glyph_Cache__Hash(Face, Codepoint);
    uint32_t SlotIndex = GLYPH_CACHE_SLOT_MASK & Hash; 
    
    glyph_slot* Slot = Cache->Slots + SlotIndex;
    
    glyph_cache_entry* TargetEntry = NULL;
    for(glyph_cache_entry* Entry = Slot->First; Entry; Entry = Entry->NextSlotEntry)
    {
        if(Entry->Face == Face && Entry->Glyph.Codepoint == Codepoint && Entry->Hash == Hash)
        {
            TargetEntry = Entry;
        }
    }
    
    if(!TargetEntry)
    {
        if(Cache->GlyphCount == Cache->BucketCount*GLYPH_CACHE_GLYPHS_PER_BUCKET)
        {
            glyph_cache_entry* CacheEntry = Cache->LRU.First;
            if(CacheEntry && (CacheEntry->Version < Cache->Version))
            {
                glyph_slot* TargetSlot = Cache->Slots + CacheEntry->SlotIndex;
                DLL_Remove_NP(Cache->LRU.First, Cache->LRU.Last, CacheEntry, NextLRUEntry, PrevLRUEntry);
                DLL_Remove_NP(TargetSlot->First, TargetSlot->Last, CacheEntry, NextSlotEntry, PrevSlotEntry);
                
                //TODO(JJ): Delete the memory region here for the atlas
                GPU_Resource_Manager_Delete_Texture2D(Cache->ResourceManager, CacheEntry->Glyph.Texture);
                TargetEntry = CacheEntry;
            }
            else
            {
                glyph_cache_bucket* Bucket = Arena_Push_Struct(Cache->Arena, glyph_cache_bucket);
                Zero_Struct(Bucket, glyph_cache_bucket);
                Cache->CurrentBucket = Bucket;
                Cache->BucketCount++;
            }
        }
        
        if(!TargetEntry)
        {  
            Assert(Cache->CurrentBucket->Count < GLYPH_CACHE_GLYPHS_PER_BUCKET);
            TargetEntry = &Cache->CurrentBucket->Glyphs[Cache->CurrentBucket->Count++];
            Cache->GlyphCount++;
        }
        
        Zero_Struct(TargetEntry, glyph_cache_entry);
        
        if(Glyph_Generator_Generate_Glyph_Metrics(Cache->Generator, &TargetEntry->Glyph, Face, Codepoint))
        {
            TargetEntry->SlotIndex = SlotIndex;
            TargetEntry->Hash      = Hash;
            DLL_Push_Back_NP(Slot->First, Slot->Last, TargetEntry, NextSlotEntry, PrevSlotEntry);
            
            glyph_generate_entry* GlyphGenerateEntry = Cache->FreeGenerateEntries;
            if(!GlyphGenerateEntry) GlyphGenerateEntry = Arena_Push_Struct(Cache->Arena, glyph_generate_entry);
            else SLL_Pop_Front(Cache->FreeGenerateEntries);
            GlyphGenerateEntry->CacheEntry = TargetEntry;
            SLL_Push_Back(Cache->GlyphGenerateQueue.First, Cache->GlyphGenerateQueue.Last, GlyphGenerateEntry);
            Cache->GlyphGenerateQueue.Count++;
            
            TargetEntry->Face = Face;
        }
    }
    else
    {
        DLL_Remove_NP(Cache->LRU.First, Cache->LRU.Last, TargetEntry, NextLRUEntry, PrevLRUEntry);
    }
    
    DLL_Push_Back_NP(Cache->LRU.First, Cache->LRU.Last, TargetEntry, NextLRUEntry, PrevLRUEntry);
    TargetEntry->Version = Cache->Version;
    return &TargetEntry->Glyph;
}

void Glyph_Cache_Generate(glyph_cache* Cache, gpu_cmd_buffer* CmdBuffer)
{
    arena* Scratch = Core_Get_Thread_Context()->Scratch;
    
    glyph_generate_queue* GenerateQueue = &Cache->GlyphGenerateQueue;
    
    uint64_t GlyphCount = 0;
    glyph** Glyphs = Arena_Push_Array(Scratch, glyph*, GenerateQueue->Count);
    glyph_bitmap* Bitmaps = Arena_Push_Array(Scratch, glyph_bitmap, GenerateQueue->Count);
    
    glyph_generate_entry* GlyphGenerateEntry = GenerateQueue->First;
    while(GlyphGenerateEntry)
    {
        glyph_cache_entry* CacheEntry = GlyphGenerateEntry->CacheEntry;
        if(Glyph_Generator_Generate_Glyph_Bitmap(Cache->Generator, Get_Base_Allocator(Scratch), &Bitmaps[GlyphCount], CacheEntry->Face, CacheEntry->Glyph.Codepoint))
            Glyphs[GlyphCount++] = &CacheEntry->Glyph;
        GlyphGenerateEntry = GlyphGenerateEntry->Next;
    }
    
    GlyphGenerateEntry = GenerateQueue->First;
    while(GlyphGenerateEntry)
    {
        glyph_generate_entry* FreeEntry = GlyphGenerateEntry;
        GlyphGenerateEntry = GlyphGenerateEntry->Next;
        SLL_Push_Front(Cache->FreeGenerateEntries, FreeEntry);
    }
    
    GenerateQueue->First = GenerateQueue->Last = NULL;
    GenerateQueue->Count = 0;
    
    for(uint64_t GlyphIndex = 0; GlyphIndex < GlyphCount; GlyphIndex++)
    {
        //TODO(JJ): Updated data to atlas or texture
        glyph* Glyph = Glyphs[GlyphIndex];
        glyph_bitmap* Bitmap = Bitmaps + GlyphIndex;
        Glyph->Texture = GPU_Resource_Manager_Create_Texture2D(Cache->ResourceManager, Bitmap->Width, Bitmap->Height, GPU_TEXTURE_FORMAT_R8G8B8A8_UNORM, GPU_TEXTURE_USAGE_SAMPLED);
        GPU_Cmd_Upload_Texture(CmdBuffer, Glyph->Texture, 0, 0, Bitmap->Texels, Bitmap->Width, Bitmap->Height);
    }
    
    Cache->Version++;
}

void Glyph_Cache_Delete(glyph_cache* Cache)
{
    Arena_Delete(Cache->Arena);
}