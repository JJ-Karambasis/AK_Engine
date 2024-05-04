internal uba_script UBA_Get_Script(SBScript Script) {
    switch(Script) {
        case SBScriptLATN: {
            return UBA_SCRIPT_LATIN;
        } break;

        case SBScriptARAB: {
            return UBA_SCRIPT_ARABIC;
        } break;

        case SBScriptDEVA: {
            return UBA_SCRIPT_DEVANAGARI;
        } break;

        Invalid_Default_Case();
    }

    return UBA_SCRIPT_NONE;
}

internal uba_direction UBA_Get_Direction(SBLevel Level) {
    //After looking around at the code, a level of 0 means LTR 
    //and 1 means RTL
    switch(Level) {
        case 0: {
            return UBA_DIRECTION_LTR;
        } break;

        case 1: {
            return UBA_DIRECTION_RTL;
        } break;

        Invalid_Default_Case();
    }

    return UBA_DIRECTION_INVALID;
}

uba* UBA_Allocate(allocator* Allocator, string Text) {
    arena* Arena = Arena_Create(Allocator);
    uba* Result = Arena_Push_Struct(Arena, uba);

    Result->Arena = Arena;
    Array_Init(&Result->Runs, Arena);
    Array_Init(&Result->Scripts, Arena);

    scratch Scratch = Scratch_Get();
    SBCodepointSequence CodepointSequence = { SBStringEncodingUTF8, (void*)Text.Str, Text.Size};
    SBScriptLocatorRef ScriptLocator = SBScriptLocatorCreate(&Scratch);
    SBScriptLocatorLoadCodepoints(ScriptLocator, &CodepointSequence);
    
    const SBScriptAgent* ScriptAgent = SBScriptLocatorGetAgent(ScriptLocator);
    while(SBScriptLocatorMoveNext(ScriptLocator)) {
        Array_Push(&Result->Scripts, {
            .Offset = ScriptAgent->offset,
            .Length = ScriptAgent->length,
            .Script = UBA_Get_Script(ScriptAgent->script)
        });
    }

    SBAlgorithmRef Algorithm = SBAlgorithmCreate(&CodepointSequence, &Scratch);
    //Iterate over paragraphs first

    array<range_u64> ParagraphRanges(&Scratch);

    SBUInteger Offset = 0;
    SBUInteger TotalLength = (SBUInteger)Text.Size;
    for(;;) {
        SBUInteger ActualLength = 0;
        SBAlgorithmGetParagraphBoundary(Algorithm, Offset, TotalLength, &ActualLength, nullptr);

        Array_Push(&ParagraphRanges, {
            .Min = Offset,
            .Max = Offset+ActualLength
        });

        Offset += ActualLength;
        TotalLength -= ActualLength;
        if(Offset == Text.Size) {
            break;
        }
    }

    for(const range_u64& Range : ParagraphRanges) {
        SBParagraphRef Paragraph = SBAlgorithmCreateParagraph(Algorithm, Range.Min, Range.Max-Range.Min, SBLevelDefaultLTR);

        Offset = SBParagraphGetOffset(Paragraph);
        TotalLength = SBParagraphGetLength(Paragraph);
        uptr End = Offset + TotalLength;

        for(;;) {
            SBLineRef Line = SBParagraphCreateLine(Paragraph, Offset, TotalLength);

            SBUInteger RunCount = SBLineGetRunCount(Line);
            const SBRun* Runs = SBLineGetRunsPtr(Line);

            for(SBUInteger i = 0; i < RunCount; i++) {
                Array_Push(&Result->Runs, {
                    .Offset = Runs[i].offset,
                    .Length = Runs[i].length,
                    .Direction = UBA_Get_Direction(Runs[i].level)
                });
            }

            SBUInteger ActualLength = SBLineGetLength(Line);

            Offset += ActualLength;
            TotalLength -= ActualLength;
            if(Offset == End) {
                break;
            }
        }

    }

    return Result;
}

span<uba_run> UBA_Get_Runs(uba* UBA) {
    return span(UBA->Runs);
}

span<uba_script_info> UBA_Get_Scripts(uba* UBA) {
    return span(UBA->Scripts);
}

void UBA_Free(uba* UBA) {
    if(UBA) {
        arena* Arena = UBA->Arena;
        Arena_Delete(Arena);
    }
}