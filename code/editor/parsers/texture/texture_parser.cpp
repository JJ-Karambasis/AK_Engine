#ifdef TEXTURE_PARSER_USE_STBI
#include "stbi/stbi_texture_parser.h"
#else
#error "Not Implemented"
#endif

#ifdef TEXTURE_PARSER_USE_STBI
#include "stbi/stbi_texture_parser.cpp"
#else
#error "Not Implemented"
#endif