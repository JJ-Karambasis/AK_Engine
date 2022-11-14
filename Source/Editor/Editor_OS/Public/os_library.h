#ifndef OS_LIBRARY_H
#define OS_LIBRARY_H

typedef struct os_library os_library;
os_library* OS_Load_Library(str8 Name);
void*       OS_Get_Symbol(os_library* Library, strc SymbolName);
void        OS_Close_Library(os_library* Library);

#endif