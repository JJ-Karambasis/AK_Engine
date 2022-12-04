#define SB_CONFIG_UNITY
#define malloc(Size) Arena_Push(Core_Get_Thread_Context()->Scratch, Size, MEMORY_NO_CLEAR)
#define free 
#include <SheenBidi.h>
#include <../Source/SheenBidi.c>
#undef malloc
#undef free

void BIDI__Push_Part(bidi* Bidi, bidi_part* Part)
{
    SLL_Push_Back(Bidi->Parts.First, Bidi->Parts.Last, Part);
    Bidi->Parts.Count++;
}

static bidi_script G__BIDI_Script[] = 
{
    BIDI_SCRIPT_UNKNOWN,
    BIDI_SCRIPT_UNKNOWN,
    BIDI_SCRIPT_COMMON,
    BIDI_SCRIPT_UNKNOWN,
    BIDI_SCRIPT_ARABIC,
    
    BIDI_SCRIPT_UNKNOWN,
    BIDI_SCRIPT_UNKNOWN,
    BIDI_SCRIPT_UNKNOWN,
    BIDI_SCRIPT_UNKNOWN,
    BIDI_SCRIPT_UNKNOWN,
    
    BIDI_SCRIPT_UNKNOWN,
    BIDI_SCRIPT_UNKNOWN,
    BIDI_SCRIPT_UNKNOWN,
    BIDI_SCRIPT_UNKNOWN,
    BIDI_SCRIPT_UNKNOWN,
    
    BIDI_SCRIPT_UNKNOWN,
    BIDI_SCRIPT_UNKNOWN,
    BIDI_SCRIPT_UNKNOWN,
    BIDI_SCRIPT_UNKNOWN,
    BIDI_SCRIPT_UNKNOWN,
    
    BIDI_SCRIPT_UNKNOWN,
    BIDI_SCRIPT_LATIN,    /**< Latin */
};

static bidi_direction G__BIDI_Level_Direction[2] = 
{
    BIDI_DIRECTION_LTR,
    BIDI_DIRECTION_RTL
};

bidi_direction BIDI__Get_Direction_From_Level(SBLevel Level)
{
    //This is according to the SBLevelAsNormalBidiType() in SheenBidi
    return G__BIDI_Level_Direction[Level&1];
}

bidi_script BIDI__Get_Script(SBScript Script)
{
    Assert(Script < Array_Count(G__BIDI_Script));
    bidi_script Result = G__BIDI_Script[Script];
    Assert(Result != BIDI_SCRIPT_UNKNOWN);
    return Result;
}

bidi BIDI_Get_Parts(arena* Arena, str8 Text)
{
    SBCodepointSequence CodepointSequence = {SBStringEncodingUTF8, (void*)Text.Str, Text.Length};
    SBAlgorithmRef BidiAlg = SBAlgorithmCreate(&CodepointSequence);
    SBScriptLocatorRef ScriptAlg = SBScriptLocatorCreate();
    SBMirrorLocatorRef MirrorAlg = SBMirrorLocatorCreate();
    
    bidi Result;
    Zero_Struct(&Result, bidi);
    
    SBUInteger CurrentOffset = 0;
    for(;;)
    {
        
        SBParagraphRef Paragraph = SBAlgorithmCreateParagraph(BidiAlg, CurrentOffset, MAX_S32, SBLevelDefaultLTR);
        if(!Paragraph) break;
        
        SBUInteger ParagraphLength = SBParagraphGetLength(Paragraph);
        SBLineRef Line = SBParagraphCreateLine(Paragraph, CurrentOffset, ParagraphLength);
        Assert(Line);
        
        SBUInteger RunCount = SBLineGetRunCount(Line);
        const SBRun* Runs = SBLineGetRunsPtr(Line);
        
        for(SBUInteger RunIndex = 0; RunIndex < RunCount; RunIndex++)
        {
            const SBRun* Run = Runs+RunIndex;
            
            bidi_part_text* PartText = Arena_Push_Struct(Arena, bidi_part_text);
            bidi_part* Part          = &PartText->Part;
            Part->Type               = BIDI_PART_TYPE_TEXT;
            Part->Offset             = Run->offset;
            Part->Length             = Run->length;
            
            str8 Str = Str8_Substr(Text, Part->Offset, Part->Offset+Part->Length);
            
            SBCodepointSequence ScriptSequence = {SBStringEncodingUTF8, (void*)Str.Str, Str.Length};
            SBScriptLocatorLoadCodepoints(ScriptAlg, &ScriptSequence);
            SBScriptLocatorMoveNext(ScriptAlg);
            const SBScriptAgent* ScriptAgent = SBScriptLocatorGetAgent(ScriptAlg);
            
            PartText->Direction      = BIDI__Get_Direction_From_Level(Run->level);
            PartText->Script         = BIDI__Get_Script(ScriptAgent->script);
            BIDI__Push_Part(&Result, Part);
        }
        
        SBMirrorLocatorLoadLine(MirrorAlg, Line, (void*)Text.Str);
        while(SBMirrorLocatorMoveNext(MirrorAlg))
        {
            const SBMirrorAgent* MirrorAgent = SBMirrorLocatorGetAgent(MirrorAlg);
            
            bidi_mirror* Mirror = Arena_Push_Struct(Arena, bidi_mirror);
            Mirror->Index     = (uint32_t)MirrorAgent->index;
            Mirror->Mirror    = MirrorAgent->mirror;
            Mirror->Codepoint = MirrorAgent->codepoint;
            
            SLL_Push_Back(Result.Mirrors.First, Result.Mirrors.Last, Mirror);
            Result.Mirrors.Count++;
        }
        
        SBUInteger SeparatorLength = 0;
        SBUInteger ParagraphOffset = SBParagraphGetOffset(Paragraph);
        SBAlgorithmGetParagraphBoundary(BidiAlg, ParagraphOffset, MAX_S32, &ParagraphLength, &SeparatorLength);
        
        if(SeparatorLength)
        {
            SBUInteger SeparatorOffset = ParagraphOffset + (ParagraphLength-SeparatorLength);
            if(Text.Str[SeparatorOffset] == '\r' || Text.Str[SeparatorOffset] == '\n')
            {
                bidi_part* Part = Arena_Push_Struct(Arena, bidi_part);
                Part->Type      = BIDI_PART_TYPE_NEWLINE;
                Part->Offset    = SeparatorOffset;
                Part->Length    = SeparatorLength;
                BIDI__Push_Part(&Result, Part);
            }
        }
        
        CurrentOffset = ParagraphOffset + ParagraphLength;
    }
    
    return Result;
}

str8 BIDI_Replace_Text(bidi* BIDI, str8 Text, arena* Arena)
{
    bidi_mirror_list* Mirrors = &BIDI->Mirrors;
    if(Mirrors->Count)
    {
        str8_list TextList;
        Zero_Struct(&TextList, str8_list);
        
        uint64_t StartIndex = 0;
        for(uint64_t TextIndex = 0; TextIndex < Text.Length; TextIndex++)
        {
            for(bidi_mirror* Mirror = Mirrors->First; Mirror; Mirror = Mirror->Next)
            {
                if(TextIndex == Mirror->Index)
                {
                    uint8_t* MirrorStrBuffer = Arena_Push_Array(Arena, uint8_t, 4);
                    str8 MirrorStr = Str8(MirrorStrBuffer, 0);
                    UTF8_From_Codepoint(Mirror->Mirror, MirrorStrBuffer, (uint32_t*)&MirrorStr.Length);
                    
                    uint32_t CodepointLength = UTF8_Get_Byte_Count(Mirror->Codepoint);
                    
                    Str8_List_Push(&TextList, Get_Base_Allocator(Arena), Str8_Substr(Text, StartIndex, Mirror->Index));
                    Str8_List_Push(&TextList, Get_Base_Allocator(Arena), MirrorStr);
                    StartIndex = Mirror->Index+CodepointLength;
                }
            }
        }
        
        Str8_List_Push(&TextList, Get_Base_Allocator(Arena), Str8_Substr(Text, StartIndex, Text.Length));
        
        Text = Str8_List_Join(Get_Base_Allocator(Arena), &TextList);
    }
    return Text;
}