#ifndef GLYPH_GENERATOR_H
#define GLYPH_GENERATOR_H

typedef struct glyph_generator glyph_generator;
#define GLYPH_GENERATOR_CREATE_FONT_FACE(name) font_face* name(glyph_generator* _Generator, const void* Data, size_t DataSize, uint32_t FaceHeight)
#define GLYPH_GENERATOR_DELETE_FONT_FACE(name) void name(glyph_generator* _Generator, font_face* _Face)
#define GLYPH_GENERATOR_DELETE(name) void name(glyph_generator* _Generator)
#define GLYPH_GENERATOR_GENERATE_GLYPH_METRICS(name) bool8_t name(glyph_generator* _Generator, glyph* Glyph, font_face* _Face, uint32_t Codepoint)
#define GLYPH_GENERATOR_GENERATE_GLYPH_BITMAP(name) bool8_t name(glyph_generator* _Generator, allocator* Allocator, glyph_bitmap* Bitmap, font_face* _Face, uint32_t Codepoint)

typedef GLYPH_GENERATOR_CREATE_FONT_FACE(glyph_generator_create_font_face);
typedef GLYPH_GENERATOR_DELETE_FONT_FACE(glyph_generator_delete_font_face);
typedef GLYPH_GENERATOR_DELETE(glyph_generator_delete);
typedef GLYPH_GENERATOR_GENERATE_GLYPH_METRICS(glyph_generator_generate_glyph_metrics);
typedef GLYPH_GENERATOR_GENERATE_GLYPH_BITMAP(glyph_generator_generate_glyph_bitmap);

#define Glyph_Generator_Create_Font_Face(GlyphGenerator, Data, DataSize, FaceHeight) (GlyphGenerator)->_VTable->Create_Font_Face(GlyphGenerator, Data, DataSize, FaceHeight)
#define Glyph_Generator_Delete_Font_Face(GlyphGenerator, Face) (GlyphGenerator)->_VTable->Delete_Font_Face(GlyphGenerator, Face)
#define Glyph_Generator_Delete(GlyphGenerator) (GlyphGenerator)->_VTable->Delete(GlyphGenerator)
#define Glyph_Generator_Generate_Glyph_Metrics(GlyphGenerator, Glyph, Face, Codepoint) (GlyphGenerator)->_VTable->Generate_Glyph_Metrics(GlyphGenerator, Glyph, Face, Codepoint)
#define Glyph_Generator_Generate_Glyph_Bitmap(GlyphGenerator, Allocator, Bitmap, Face, Codepoint) (GlyphGenerator)->_VTable->Generate_Glyph_Bitmap(GlyphGenerator, Allocator, Bitmap, Face, Codepoint)

typedef struct glyph_generator_vtable
{
    glyph_generator_create_font_face*       Create_Font_Face;
    glyph_generator_delete_font_face*       Delete_Font_Face;
    glyph_generator_delete*                 Delete;
    glyph_generator_generate_glyph_metrics* Generate_Glyph_Metrics;
    glyph_generator_generate_glyph_bitmap*  Generate_Glyph_Bitmap;
} glyph_generator_vtable;

typedef struct glyph_generator
{
    glyph_generator_vtable* _VTable;
} glyph_generator;

#ifndef NO_GLYPH_GENERATOR
#include "ft/ft_glyph_generator.h"
#endif

#endif