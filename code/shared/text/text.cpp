
#ifdef TEXT_FREETYPE_FONT_MANAGER
#include "glyph_manager/freetype/freetype_manager.h"
#include "glyph_manager/freetype/freetype_manager.cpp"
#endif

//Freetype has conflicts with our core library so we need to include core
//after freetype
#include <core/core.h>
#include <text/text.h>

#ifdef TEXT_UBA_SHEENBIDI
#include "uba/sheenbidi/sheenbidi_uba.h"
#include "uba/sheenbidi/sheenbidi_uba.cpp"
#endif

#ifdef TEXT_SHAPER_HARFBUZZ
//#include "text_shaper/harfbuzz/harfbuzz_text_shaper.h"
//#include "text_shaper/harfbuzz/harfbuzz_text_shaper.cpp"
#endif