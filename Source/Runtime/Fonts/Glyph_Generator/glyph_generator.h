#ifndef GLYPH_GENERATOR_H
#define GLYPH_GENERATOR_H

#define GLYPH_GENERATOR_CREATE_GLYPH_FACE(name) glyph_face* name(glyph_generator* _Generator, const void* Data, size_t DataSize)
#define GLYPH_GENERATOR_DELETE_GLYPH_FACE(name) void name(glyph_generator* _Generator, glyph_face* _Face)
#define GLYPH_GENERATOR_DELETE(name) void name(glyph_generator* _Generator)

typedef GLYPH_GENERATOR_CREATE_GLYPH_FACE(glyph_generator_create_glyph_face);
typedef GLYPH_GENERATOR_DELETE_GLYPH_FACE(glyph_generator_delete_glyph_face);
typedef GLYPH_GENERATOR_DELETE(glyph_generator_delete);

#define Glyph_Generator_Create_Glyph_Face(Generator, Data, DataSize) (Generator)->_VTable->Create_Glyph_Face(Generator, Data, DataSize)
#define Glyph_Generator_Delete_Glyph_Face(Generator, Face) (Generator)->_VTable->Delete_Glyph_Face(Generator, Face)
#define Glyph_Generator_Delete(Generator) (Generator)->_VTable->Delete(Generator)

typedef struct glyph_generator_vtable
{
    glyph_generator_create_glyph_face* Create_Glyph_Face;
    glyph_generator_delete_glyph_face* Delete_Glyph_Face;
    glyph_generator_delete*            Delete;
} glyph_generator_vtable;

typedef struct glyph_generator
{
    glyph_generator_vtable* _VTable;
} glyph_generator;

typedef struct glyph_bitmap
{
    uint32_t* Texels;
    uint32_t  Width;
    uint32_t  Height;
} glyph_bitmap;

typedef struct glyph_face_v_metrics
{
    int32_t Ascender;
    int32_t Descender;
    int32_t LineGap;
} glyph_face_v_metrics;

typedef struct glyph_metrics
{
    int32_t XBearing;
    int32_t YBearing;
    int32_t Width;
    int32_t Height;
} glyph_metrics;

typedef struct glyph_pt
{
    int32_t x;
    int32_t y;
} glyph_pt;

typedef struct glyph_outline
{
    glyph_pt* Ptr;
    uint64_t  Count;
} glyph_outline;

#define GLYPH_FACE_SET_PIXEL_SIZE(name) void name(glyph_face* _Face, uint32_t PixelSize)
#define GLYPH_FACE_GENERATE_GLYPH_BITMAP(name) glyph_bitmap name(glyph_face* _Face, allocator* Allocator, uint32_t Codepoint)
#define GLYPH_FACE_GET_CHAR_INDEX(name) uint32_t name(glyph_face* _Face, uint32_t Codepoint)
#define GLYPH_FACE_GET_CHAR_VARIANT_INDEX(name) uint32_t name(glyph_face* _Face, uint32_t Codepoint, uint32_t Variant)
#define GLYPH_FACE_GET_V_METRICS(name) bool8_t name(glyph_face* _Face, glyph_face_v_metrics* Metrics)
#define GLYPH_FACE_GET_GLYPH_H_ADVANCE(name) uint32_t name(glyph_face* _Face, uint32_t Codepoint)
#define GLYPH_FACE_GET_KERNING(name) uint32_t name(glyph_face* _Face, uint32_t LeftCodepoint, uint32_t RightCodepoint)
#define GLYPH_FACE_GET_GLYPH_METRICS(name) bool8_t name(glyph_face* _Face, uint32_t Codepoint, glyph_metrics* Metrics)
#define GLYPH_FACE_GET_GLYPH_OUTLINE(name) glyph_outline name(glyph_face* _Face, allocator* Allocator, uint32_t Codepoint)

typedef GLYPH_FACE_SET_PIXEL_SIZE(glyph_face_set_pixel_size);
typedef GLYPH_FACE_GENERATE_GLYPH_BITMAP(glyph_face_generate_glyph_bitmap);
typedef GLYPH_FACE_GET_CHAR_INDEX(glyph_face_get_char_index);
typedef GLYPH_FACE_GET_CHAR_VARIANT_INDEX(glyph_face_get_char_variant_index);
typedef GLYPH_FACE_GET_V_METRICS(glyph_face_get_v_metrics);
typedef GLYPH_FACE_GET_GLYPH_H_ADVANCE(glyph_face_get_glyph_h_advance);
typedef GLYPH_FACE_GET_KERNING(glyph_face_get_kerning);
typedef GLYPH_FACE_GET_GLYPH_METRICS(glyph_face_get_glyph_metrics);
typedef GLYPH_FACE_GET_GLYPH_OUTLINE(glyph_face_get_glyph_outline);

#define Glyph_Face_Set_Pixel_Size(Face, PixelSize) (Face)->_VTable->Set_Pixel_Sizes(Face, PixelSize)
#define Glyph_Face_Generate_Glyph_Bitmap(Face, Allocator, Codepoint) (Face)->_VTable->Generate_Glyph_Bitmap(Face, Allocator, Codepoint)
#define Glyph_Face_Get_Char_Index(Face, Codepoint) (Face)->_VTable->Get_Char_Index(Face, Codepoint)
#define Glyph_Face_Get_Char_Variant_Index(Face, Codepoint, Variant) (Face)->_VTable->Get_Char_Variant_Index(Face, Codepoint, Variant)
#define Glyph_Face_Get_V_Metrics(Face, Metrics) (Face)->_VTable->Get_V_Metrics(Face, Metrics)
#define Glyph_Face_Get_Glyph_H_Advance(Face, Codepoint) (Face)->_VTable->Get_Glyph_H_Advance(Face, Codepoint)
#define Glyph_Face_Get_Kerning(Face, LeftCodepoint, RightCodepoint) (Face)->_VTable->Get_Kerning(Face, LeftCodepoint, RightCodepoint)
#define Glyph_Face_Get_Glyph_Metrics(Face, Codepoint, Metrics) (Face)->_VTable->Get_Glyph_Metrics(Face, Codepoint, Metrics)
#define Glyph_Face_Get_Glyph_Outline(Face, Allocator, Codepoint) (Face)->_VTable->Get_Glyph_Outline(Face, Allocator, Codepoint)

typedef struct glyph_face_vtable
{
    glyph_face_set_pixel_size*         Set_Pixel_Sizes;
    glyph_face_generate_glyph_bitmap*  Generate_Glyph_Bitmap;
    glyph_face_get_char_index*         Get_Char_Index;
    glyph_face_get_char_variant_index* Get_Char_Variant_Index;
    glyph_face_get_v_metrics*          Get_V_Metrics;
    glyph_face_get_glyph_h_advance*    Get_Glyph_H_Advance;
    glyph_face_get_kerning*            Get_Kerning;
    glyph_face_get_glyph_metrics*      Get_Glyph_Metrics;
    glyph_face_get_glyph_outline*      Get_Glyph_Outline;
} glyph_face_vtable;

typedef struct glyph_face
{
    glyph_face_vtable* _VTable;
    const_buffer        Buffer;
} glyph_face;

#include "ft/ft_glyph_generator.h"

#endif