#ifndef IM_STACK_H
#define IM_STACK_H


namespace im_stack_type {
    using bind_group_indices = static_array<u32, RENDERER_MAX_BIND_GROUP>;
    
    template <typename array>
    static constexpr inline array Compute_Indices_Offset(u32 LastIndex) {
        u32 CurrentIndex = LastIndex+1;
        array Result = {};
        for(uptr i = 0; i < Result.Count(); i++) {
            Result[i] = CurrentIndex++;
        }
        return Result;
    }

    static constexpr inline u32 Compute_Index_Offset(u32 LastIndex) {
        return LastIndex+1;
    }

    static const u32 Pipeline = 0;
    static const bind_group_indices BindGroups = Compute_Indices_Offset<bind_group_indices>(Pipeline);
    static const u32 Count = Compute_Index_Offset(Array_Last(&BindGroups));
};

struct im_stack_entry {
    im_stack_entry* Next;
    im_stack_entry* Prev;
};

struct im_stack_list {
    im_stack_entry* First;
    im_stack_entry* Last;
    b32             AutoPop;
};

struct im_stack_pipeline : im_stack_entry {
    gdi_handle<gdi_pipeline> Value;
};

struct im_stack_bind_group : im_stack_entry {
    gdi_handle<gdi_bind_group> Value;
};


namespace im_stack_size {
    using bind_group_size = static_array<uptr, RENDERER_MAX_BIND_GROUP>;

    static_array<uptr, 2+RENDERER_MAX_BIND_GROUP> Sizes;
    struct array_iter {
        static_array<uptr, 2+RENDERER_MAX_BIND_GROUP>* Sizes;
        u32 Index;
    };

    static inline uptr Compute_Size(array_iter* Iter, uptr Size) {
        Iter->Sizes->At(Iter->Index++) = Size;
        return Size;
    }

    template <typename array>
    static inline array Compute_Array_Size(array_iter* Iter, uptr Size) {
        array Result = {};
        for(uptr i = 0; i < Result.Count(); i++) {
            Iter->Sizes->At(Iter->Index++) = Size;
            Result[i] = Size;
        }
        return Result;
    }
    
    array_iter Iter = {&Sizes, 0};
    static const uptr Pipeline = Compute_Size(&Iter, sizeof(im_stack_pipeline));
    static const bind_group_size BindGroups = Compute_Array_Size<bind_group_size>(&Iter, sizeof(im_stack_bind_group));
};

#endif