internal atlas_allocator_free_list_type Atlas__Get_Free_List_Type(s32 SmallThreshold, s32 LargeThreshold, dim2i Size) {
    if(Size.width >= LargeThreshold || Size.height >= LargeThreshold) {
        return ATLAS_ALLOCATOR_FREE_LIST_TYPE_LARGE;
    } else if(Size.width >= SmallThreshold || Size.height >= SmallThreshold) {
        return ATLAS_ALLOCATOR_FREE_LIST_TYPE_MEDIUM;
    } else {
        return ATLAS_ALLOCATOR_FREE_LIST_TYPE_SMALL;
    }
}

internal atlas_allocator_node* Atlas__Create_Node(atlas_allocator* Allocator) {
    atlas_allocator_node* Result = nullptr;
    if(Allocator->OrphanList.Count) {
        Result = Array_Pop(&Allocator->OrphanList);
        Assert(Result->Type == ATLAS_ALLOCATOR_NODE_TYPE_NONE);
    } else {
        Result = Arena_Push_Struct(Allocator->Arena, atlas_allocator_node);
        Result->Type = ATLAS_ALLOCATOR_NODE_TYPE_NONE;
#ifdef DEBUG_BUILD
        SLL_Push_Front_N(Allocator->AllocatedList, Result, NextInList);
#endif    
    }
    Result->Generation++;
    return Result;
}

internal void Atlas__Delete_Node(atlas_allocator* Allocator, atlas_allocator_node* Node) {
    Node->Type = ATLAS_ALLOCATOR_NODE_TYPE_NONE;
    Node->Next = nullptr;
    Array_Push(&Allocator->OrphanList, Node);
}

internal void Atlas__Add_Free_Rect(atlas_allocator* Allocator, atlas_allocator_node* Node, dim2i Size) {
    Assert(Node->Type == ATLAS_ALLOCATOR_NODE_TYPE_FREE);
    atlas_allocator_free_list_type Type = Atlas__Get_Free_List_Type(Allocator->SmallThreshold, Allocator->LargeThreshold, Size);
    Array_Push(&Allocator->FreeLists[Type], Node);
}

internal atlas_allocator_node* Atlas__Find_Free_Rect(atlas_allocator* Allocator, dim2i RequestSize) {
    atlas_allocator_free_list_type ListType = Atlas__Get_Free_List_Type(Allocator->SmallThreshold, Allocator->LargeThreshold, RequestSize);
    bool UseWorstFit = ListType == ATLAS_ALLOCATOR_FREE_LIST_TYPE_LARGE;

    for(u32 ListIndex = ListType; ListIndex < ATLAS_ALLOCATOR_FREE_LIST_TYPE_COUNT; ListIndex++) {
        s32 BestScore = UseWorstFit ? 0 : INT32_MAX;
        uptr BestIndex = (uptr)-1;
        atlas_allocator_node* BestNode = nullptr;

        array<atlas_allocator_node*>& FreeList = Allocator->FreeLists[ListIndex];
        uptr FreeListIndex = 0;
        while(FreeListIndex < FreeList.Count) {
            atlas_allocator_node* FreeNode = FreeList[FreeListIndex];
            if(FreeNode->Type != ATLAS_ALLOCATOR_NODE_TYPE_FREE) {
                Array_Swap_Remove(&FreeList, FreeListIndex);
                continue;
            }

            Assert(FreeNode->Type == ATLAS_ALLOCATOR_NODE_TYPE_FREE);

            dim2i Size = Rect2i_Get_Dim(FreeNode->Rect);

            s32 dx = Size.width  - RequestSize.width;
            s32 dy = Size.height - RequestSize.height;

            if(dx >= 0 && dy >= 0) {
                if(dx == 0 || dy == 0) {
                    //Perfect fit!
                    BestNode = FreeNode;
                    BestIndex = FreeListIndex;
                    break;
                }

                s32 Score = Min(dx, dy);
                if((UseWorstFit && (Score > BestScore)) ||
                   (!UseWorstFit && (Score < BestScore))) {
                    BestScore = Score;
                    BestNode = FreeNode;
                    BestIndex = FreeListIndex;
                }
            }

            FreeListIndex++;
        }

        if(BestNode) {
            Array_Swap_Remove(&FreeList, BestIndex);
            return BestNode;
        }
    }

    return nullptr;

}

internal void Atlas__Merge_Siblings(atlas_allocator* Allocator, atlas_allocator_node* Node, atlas_allocator_node* Next, b32 IsVertical) {
    Assert(Node->Type == ATLAS_ALLOCATOR_NODE_TYPE_FREE);
    Assert(Next->Type == ATLAS_ALLOCATOR_NODE_TYPE_FREE);
    rect2i R1 = Node->Rect;
    rect2i R2 = Next->Rect;

    //Merge the nodes based on alignment
    dim2i MergeSize = Rect2i_Get_Dim(R2);
    if(IsVertical) {
        Assert(R1.P1.x == R2.P1.x);
        Assert(R1.P2.x == R2.P2.x);
        Node->Rect.P2.y += MergeSize.height;
    } else {
        Assert(R1.P1.y == R2.P1.y);
        Assert(R1.P2.y == R2.P2.y);
        Node->Rect.P2.x += MergeSize.width;
    }

    //Remove the merged node from the sibling list
    atlas_allocator_node* NextNext = Next->Next;
    Node->Next = NextNext;
    if(NextNext) {
        NextNext->Prev = Node;
    }

    //Change its state so it can be cleaned up later
    Atlas__Delete_Node(Allocator, Next);
}

internal void Atlas__Guillotine_Rect(rect2i* ChosenRect, dim2i RequestSize, b32 IsVertical, rect2i* OutSplitRect, rect2i* OutLeftoverRect, b32* OutIsVertical) {
    rect2i LeftoverRectToRight(
        ChosenRect->P1 + dim2i(RequestSize.width, 0),
        point2i(ChosenRect->P2.x, ChosenRect->P1.y+RequestSize.height)
    );

    rect2i LeftoverRectToBottom(
        ChosenRect->P1 + dim2i(0, RequestSize.height),
        point2i(ChosenRect->P1.x + RequestSize.width, ChosenRect->P2.y)
    );

    if(RequestSize == Rect2i_Get_Dim(*ChosenRect)) {
        *OutIsVertical = IsVertical;
        *OutSplitRect = rect2i();
        *OutLeftoverRect = rect2i();
    } else if(Rect2i_Area(LeftoverRectToRight) > Rect2i_Area(LeftoverRectToBottom)) {
        *OutLeftoverRect = LeftoverRectToBottom;
        *OutSplitRect = rect2i(
            LeftoverRectToRight.P1,
            point2i(LeftoverRectToRight.P2.x, ChosenRect->P2.y)
        );
        *OutIsVertical = false;
    } else {
        *OutLeftoverRect = LeftoverRectToRight;
        *OutSplitRect = rect2i(
            LeftoverRectToBottom.P1,
            point2i(ChosenRect->P2.x, LeftoverRectToBottom.P2.y)
        );
        *OutIsVertical = true;
    }
}

#ifdef DEBUG_BUILD

void Atlas__Check_Siblings(atlas_allocator_node* Node, atlas_allocator_node* Next, b32 IsVertical) {
    if(!Next) return;
    Assert(Node == Next->Prev);

    if(Node->Type == ATLAS_ALLOCATOR_NODE_TYPE_NONE || Node->Type == ATLAS_ALLOCATOR_NODE_TYPE_CONTAINER) {
        return;
    }

    if(Next->Type == ATLAS_ALLOCATOR_NODE_TYPE_NONE || Next->Type == ATLAS_ALLOCATOR_NODE_TYPE_CONTAINER) {
        return;
    }

    rect2i R1 = Node->Rect;
    rect2i R2 = Next->Rect;
    if(IsVertical) {
        Assert(R1.P1.x == R2.P1.x);
        Assert(R1.P2.x == R2.P2.x);
    } else {
        Assert(R1.P1.y == R2.P1.y);
        Assert(R1.P2.y == R2.P2.y);
    }
}

void Atlas__Check_Tree(atlas_allocator* Allocator) {
    atlas_allocator_node* Node = Allocator->AllocatedList;
    while(Node) {
        if(Node->Type == ATLAS_ALLOCATOR_NODE_TYPE_NONE) {
            Node = Node->NextInList;
            continue;
        }

        atlas_allocator_node* Iter = Node->Next;
        while(Iter) {
            Assert(Iter->IsVertical == Node->IsVertical);
            Assert(Iter->Parent == Node->Parent);
            Assert(Iter->Type != ATLAS_ALLOCATOR_NODE_TYPE_NONE);

            atlas_allocator_node* Next = Iter->Next;
            Atlas__Check_Siblings(Iter, Next, Node->IsVertical);

            Iter = Next;
        }

        if(Node->Parent) {
            Assert(Node->Parent->Type == ATLAS_ALLOCATOR_NODE_TYPE_CONTAINER);
            Assert(Node->Parent->IsVertical == !Node->IsVertical);
        }

        Node = Node->NextInList;
    }
}
#endif

atlas_allocator* Atlas_Allocator_Create(const atlas_allocator_create_info& CreateInfo) {
    Assert(CreateInfo.Allocator);
    Assert(CreateInfo.Dim.width > 0);
    Assert(CreateInfo.Dim.height > 0);
    Assert(CreateInfo.Alignment.x > 0);
    Assert(CreateInfo.Alignment.y > 0);
    Assert(CreateInfo.LargeSizeThreshold >= CreateInfo.SmallSizeThreshold);
    
    arena* Arena = Arena_Create(CreateInfo.Allocator);

    atlas_allocator* Result = Arena_Push_Struct(Arena, atlas_allocator);
    Result->Arena = Arena;
    Result->Size = CreateInfo.Dim;
    Result->Alignment = CreateInfo.Alignment;
    Result->SmallThreshold = CreateInfo.SmallSizeThreshold;
    Result->LargeThreshold = CreateInfo.LargeSizeThreshold;
    for(u32 i = 0; i < ATLAS_ALLOCATOR_FREE_LIST_TYPE_COUNT; i++) {
        Array_Init(&Result->FreeLists[i], Arena);
    }
    Array_Init(&Result->OrphanList, Arena);

    atlas_allocator_node* Node = Atlas__Create_Node(Result);
    Node->Rect                 = Rect2i_From_Dim(Result->Size);
    Node->IsVertical           = true;
    Node->Type                 = ATLAS_ALLOCATOR_NODE_TYPE_FREE;
    Atlas__Add_Free_Rect(Result, Node, Result->Size);
    Result->RootNode = Node;

    return Result;
}

void             Atlas_Allocator_Delete(atlas_allocator* Allocator);

atlas_alloc_id Atlas_Allocator_Alloc(atlas_allocator* Allocator, dim2i Dim) {
    if(Dim.width == 0 || Dim.height == 0) {
        //Cannot allocate into the atlas if we don't request any space
        return {};
    }

    Dim.width = Align(Allocator->Alignment.x, Dim.width);
    Dim.height = Align(Allocator->Alignment.y, Dim.height);
    
    atlas_allocator_node* Node = Atlas__Find_Free_Rect(Allocator, Dim);
    if(!Node) {
        //Cannot find a free rect. Atlas is full!
        return {};
    }

    rect2i Rect(Node->Rect.P1, Node->Rect.P1+Dim);
    Assert(Node->Type == ATLAS_ALLOCATOR_NODE_TYPE_FREE);

    rect2i SplitRect;
    rect2i LeftoverRect;
    b32    IsVertical;
    Atlas__Guillotine_Rect(&Node->Rect, Dim, Node->IsVertical, &SplitRect, &LeftoverRect, &IsVertical);

    atlas_allocator_node* AllocatedNode = nullptr;
    atlas_allocator_node* SplitNode = nullptr;
    atlas_allocator_node* LeftoverNode = nullptr;

    if(IsVertical == Node->IsVertical) {
        if(!Rect2i_Is_Empty(SplitRect)) {
            atlas_allocator_node* NextSibling = Node->Next;
            SplitNode = Atlas__Create_Node(Allocator);

            SplitNode->Parent     = Node->Parent;
            SplitNode->Next       = NextSibling;
            SplitNode->Prev       = Node;
            SplitNode->Rect       = SplitRect;
            SplitNode->IsVertical = Node->IsVertical;
            SplitNode->Type       = ATLAS_ALLOCATOR_NODE_TYPE_FREE;

            Node->Next = SplitNode;
            if(NextSibling) {
                NextSibling->Prev = SplitNode;
            }
        }

        if(!Rect2i_Is_Empty(LeftoverRect)) {
            Node->Type = ATLAS_ALLOCATOR_NODE_TYPE_CONTAINER;

            AllocatedNode = Atlas__Create_Node(Allocator);
            LeftoverNode  = Atlas__Create_Node(Allocator);

            AllocatedNode->Parent = Node;
            AllocatedNode->Next = LeftoverNode;
            AllocatedNode->Prev = nullptr;
            AllocatedNode->Rect = Rect;
            AllocatedNode->Type = ATLAS_ALLOCATOR_NODE_TYPE_ALLOC;
            AllocatedNode->IsVertical  = !Node->IsVertical;

            LeftoverNode->Parent = Node;
            LeftoverNode->Next = nullptr;
            LeftoverNode->Prev = AllocatedNode;
            LeftoverNode->Rect = LeftoverRect;
            LeftoverNode->Type = ATLAS_ALLOCATOR_NODE_TYPE_FREE;
            LeftoverNode->IsVertical  = !Node->IsVertical;
        } else {
            AllocatedNode = Node;
            AllocatedNode->Type = ATLAS_ALLOCATOR_NODE_TYPE_ALLOC;
            AllocatedNode->Rect = Rect;
        }
    } else {
        Node->Type = ATLAS_ALLOCATOR_NODE_TYPE_CONTAINER;

        if(!Rect2i_Is_Empty(SplitRect)) {
            SplitNode = Atlas__Create_Node(Allocator);
            SplitNode->Parent = Node;
            SplitNode->Next = nullptr;
            SplitNode->Prev = nullptr;
            SplitNode->Rect = SplitRect;
            SplitNode->Type = ATLAS_ALLOCATOR_NODE_TYPE_FREE;
            SplitNode->IsVertical = !Node->IsVertical;
        }

        if(!Rect2i_Is_Empty(LeftoverRect)) {
            Assert(SplitNode);
            atlas_allocator_node* ContainerNode = Atlas__Create_Node(Allocator);
            ContainerNode->Parent = Node;
            ContainerNode->Next = SplitNode;
            ContainerNode->Prev = nullptr;
            ContainerNode->Rect = rect2i();
            ContainerNode->Type = ATLAS_ALLOCATOR_NODE_TYPE_CONTAINER;
            ContainerNode->IsVertical = !Node->IsVertical;

            SplitNode->Prev = ContainerNode;

            AllocatedNode = Atlas__Create_Node(Allocator);
            LeftoverNode  = Atlas__Create_Node(Allocator);

            AllocatedNode->Parent = ContainerNode;
            AllocatedNode->Next = LeftoverNode;
            AllocatedNode->Prev = nullptr;
            AllocatedNode->Rect = Rect;
            AllocatedNode->Type = ATLAS_ALLOCATOR_NODE_TYPE_ALLOC;
            AllocatedNode->IsVertical = Node->IsVertical;

            LeftoverNode->Parent = ContainerNode;
            LeftoverNode->Next = nullptr;
            LeftoverNode->Prev = AllocatedNode;
            LeftoverNode->Rect = LeftoverRect;
            LeftoverNode->Type = ATLAS_ALLOCATOR_NODE_TYPE_FREE;
            LeftoverNode->IsVertical = Node->IsVertical;
        } else {
            AllocatedNode = Atlas__Create_Node(Allocator);
            AllocatedNode->Parent = Node;
            AllocatedNode->Next   = SplitNode;
            AllocatedNode->Prev   = nullptr;
            AllocatedNode->Rect   = Rect;
            AllocatedNode->Type   = ATLAS_ALLOCATOR_NODE_TYPE_ALLOC;
            AllocatedNode->IsVertical = !Node->IsVertical;

            SplitNode->Prev = AllocatedNode;
        }
    }

    Assert(AllocatedNode->Type == ATLAS_ALLOCATOR_NODE_TYPE_ALLOC);
    if(SplitNode) {
        Atlas__Add_Free_Rect(Allocator, SplitNode, Rect2i_Get_Dim(SplitRect));
    }

    if(LeftoverNode) {
        Atlas__Add_Free_Rect(Allocator, LeftoverNode, Rect2i_Get_Dim(LeftoverRect));
    }

#ifdef DEBUG_BUILD
    Atlas__Check_Tree(Allocator);
#endif

    return {
        .Generation = AllocatedNode->Generation,
        .Index      = AllocatedNode
    };
}

void Atlas_Allocator_Free(atlas_allocator* Allocator, atlas_alloc_id AllocID) {
    atlas_allocator_node* Node = (atlas_allocator_node*)Atlas_Allocator_Get(Allocator, AllocID);
    if(Node) {
        Assert(Node->Type == ATLAS_ALLOCATOR_NODE_TYPE_ALLOC);
        Node->Type = ATLAS_ALLOCATOR_NODE_TYPE_FREE;

        for(;;) {
            b32 IsVertical = Node->IsVertical;
            
            atlas_allocator_node* Next = Node->Next;
            atlas_allocator_node* Prev = Node->Prev;

            if(Next && Next->Type == ATLAS_ALLOCATOR_NODE_TYPE_FREE) {
                Atlas__Merge_Siblings(Allocator, Node, Next, IsVertical);
            }

            if(Prev && Prev->Type == ATLAS_ALLOCATOR_NODE_TYPE_FREE) {
                Atlas__Merge_Siblings(Allocator, Prev, Node, IsVertical);
                Node = Prev;
            }

            //If the node is a unique child. We try and collapse it
            atlas_allocator_node* Parent = Node->Parent;
            if(!Node->Prev && !Node->Next && Parent) {
                Assert(Parent->Type == ATLAS_ALLOCATOR_NODE_TYPE_CONTAINER);

                Parent->Rect = Node->Rect;
                Parent->Type = ATLAS_ALLOCATOR_NODE_TYPE_FREE;
                Atlas__Delete_Node(Allocator, Node);
                Node = Parent;
            } else {
                Atlas__Add_Free_Rect(Allocator, Node, Rect2i_Get_Dim(Node->Rect));
                break;
            }
        }

#ifdef DEBUG_BUILD
        Atlas__Check_Tree(Allocator);
#endif
    }
}

atlas_index* Atlas_Allocator_Get(atlas_allocator* Allocator, atlas_alloc_id AllocID) {
    if(!AllocID.Generation || !AllocID.Index) return nullptr; 
    
    atlas_allocator_node* Node = (atlas_allocator_node*)AllocID.Index;
    if((Node->Type != ATLAS_ALLOCATOR_NODE_TYPE_ALLOC) || 
       (Node->Generation != AllocID.Generation)) {
        return nullptr;
    }

    return AllocID.Index;
}