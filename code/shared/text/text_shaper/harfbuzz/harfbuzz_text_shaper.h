#ifndef HARFBUZZ_TEXT_SHAPER_H
#define HARFBUZZ_TEXT_SHAPER_H

#include <hb.h>

struct text_shaper_face {
    hb_font_t*    Font;
};

struct text_shaper_buffer {
    hb_buffer_t*        Buffer;
    text_shaper_buffer* Next;
};

struct text_shaper {
    arena*                       Arena;
    ak_mutex                     Mutex;
    async_pool<text_shaper_face> FacePool;
    text_shaper_buffer*          FreeBufferList;
};

#endif