uint32_t Glyph_Cache__Hash(uint32_t Codepoint, uint32_t PixelHeight, glyph_face* Face)
{
    uint32_t Result = Hash_Combine(Hash_Combine(Hash_U32(Codepoint), Hash_U32(PixelHeight)), Hash_Ptr((size_t)Face));
    return Result;
}

glyph_cache* Glyph_Cache_Create(allocator* Allocator, gpu_resource_manager* ResourceManager, 
                                uint32_t SlotCapacity)
{
    arena* Arena = Arena_Create(Allocator, Mega(1));
    glyph_cache* Result = Arena_Push_Struct(Arena, glyph_cache);
    Result->Arena = Arena;
    Result->ResourceManager = ResourceManager;
    Result->SlotCapacity = Ceil_Pow2_U32(SlotCapacity);
    Result->Slots = Arena_Push_Array(Arena, glyph_slot, Result->SlotCapacity);
    return Result;
}

const glyph* Glyph_Cache_Get_Glyph(glyph_cache* Cache, glyph_face* Face, uint32_t Codepoint, uint32_t PixelHeight)
{
    uint32_t SlotMask = Cache->SlotCapacity-1;
    
    Glyph_Face_Set_Pixel_Size(Face, PixelHeight);
    
    uint32_t Hash = Glyph_Cache__Hash(Codepoint, PixelHeight, Face);
    uint32_t SlotIndex = SlotMask & Hash; 
    
    glyph_slot* Slot = Cache->Slots + SlotIndex;
    
    glyph_cache_entry* TargetEntry = NULL;
    for(glyph_cache_entry* Entry = Slot->First; Entry; Entry = Entry->NextSlotEntry)
    {
        if(Entry->Face == Face && Entry->PixelHeight == PixelHeight && Entry->Glyph.Codepoint == Codepoint && Entry->Hash == Hash)
        {
            TargetEntry = Entry;
        }
    }
    
    if(!TargetEntry)
    {
        glyph_metrics Metrics;
        if(Glyph_Face_Get_Glyph_Metrics(Face, Codepoint, &Metrics))
        {
            if(Cache->GlyphCount == Cache->BucketCount*GLYPH_CACHE_GLYPHS_PER_BUCKET)
            {
                glyph_cache_bucket* Bucket = Arena_Push_Struct(Cache->Arena, glyph_cache_bucket);
                Cache->CurrentBucket = Bucket;
                Cache->BucketCount++;
            }
            
            Assert(Cache->CurrentBucket->Count < GLYPH_CACHE_GLYPHS_PER_BUCKET);
            TargetEntry = &Cache->CurrentBucket->Glyphs[Cache->CurrentBucket->Count++];
            Cache->GlyphCount++;
            
            Zero_Struct(TargetEntry, glyph_cache_entry);
            
            TargetEntry->Glyph.Codepoint = Codepoint;
            TargetEntry->Glyph.XBearing  = Metrics.XBearing;
            TargetEntry->Glyph.YBearing  = Metrics.YBearing;
            TargetEntry->Face            = Face;
            TargetEntry->PixelHeight     = PixelHeight;
            TargetEntry->SlotIndex       = SlotIndex;
            TargetEntry->Hash            = Hash;
            DLL_Push_Back_NP(Slot->First, Slot->Last, TargetEntry, NextSlotEntry, PrevSlotEntry);
            
            glyph_generate_entry* GenerateEntry = Cache->FreeGenerateEntries;
            if(!GenerateEntry) GenerateEntry = Arena_Push_Struct(Cache->Arena, glyph_generate_entry);
            else SLL_Pop_Front(Cache->FreeGenerateEntries);
            GenerateEntry->CacheEntry = TargetEntry;
            SLL_Push_Back(Cache->GenerateQueue.First, Cache->GenerateQueue.Last, GenerateEntry);
            Cache->GenerateQueue.Count++;
        }
    }
    
    if(TargetEntry) 
        return &TargetEntry->Glyph;
    
    return NULL;
}

void Glyph_Cache_Generate(glyph_cache* Cache, gpu_cmd_buffer* CmdBuffer)
{
    arena* Scratch = Core_Get_Thread_Context()->Scratch;
    
    glyph_generate_queue* GenerateQueue = &Cache->GenerateQueue;
    
    uint64_t GlyphCount = 0;
    glyph** Glyphs = Arena_Push_Array(Scratch, glyph*, GenerateQueue->Count);
    glyph_bitmap* Bitmaps = Arena_Push_Array(Scratch, glyph_bitmap, GenerateQueue->Count);
    
    glyph_generate_entry* GlyphGenerateEntry = GenerateQueue->First;
    while(GlyphGenerateEntry)
    {
        glyph_cache_entry* CacheEntry = GlyphGenerateEntry->CacheEntry;
        
        Glyph_Face_Set_Pixel_Size(CacheEntry->Face, CacheEntry->PixelHeight);
        Bitmaps[GlyphCount] = Glyph_Face_Generate_Glyph_Bitmap(CacheEntry->Face, Get_Base_Allocator(Scratch), CacheEntry->Glyph.Codepoint);
        if(Bitmaps[GlyphCount].Texels)
        {
            Glyphs[GlyphCount++] = &CacheEntry->Glyph;
            GlyphGenerateEntry = GlyphGenerateEntry->Next;
        }
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
        Glyph->Texture.Texture = GPU_Resource_Manager_Create_Texture2D(Cache->ResourceManager, Bitmap->Width, Bitmap->Height, GPU_TEXTURE_FORMAT_R8G8B8A8_UNORM, GPU_TEXTURE_USAGE_SAMPLED);
        Glyph->Texture.Width  = Bitmap->Width;
        Glyph->Texture.Height = Bitmap->Height;
        GPU_Cmd_Upload_Texture(CmdBuffer, Glyph->Texture.Texture, 0, 0, Bitmap->Texels, Bitmap->Width, Bitmap->Height);
    }
    
}

void Glyph_Cache_Delete(glyph_cache* Cache)
{
    Arena_Delete(Cache->Arena);
}