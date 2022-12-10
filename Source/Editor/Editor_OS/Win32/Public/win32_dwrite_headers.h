#ifndef WIN32_DWRITE_HEADERS_H
#define WIN32_DWRITE_HEADERS_H

static const GUID IID_IDWriteFactory =
{0xb859ee5a,0xd838,0x4b5b,{0xa2,0xe8,0x1a,0xdc,0x7d,0x93,0xdb,0x48}};

typedef struct IDWriteFactory IDWriteFactory;
typedef struct IDWriteFontCollection IDWriteFontCollection;
typedef struct IDWriteFontFamily IDWriteFontFamily;
typedef struct IDWriteLocalizedStrings IDWriteLocalizedStrings;
typedef struct IDWriteFont IDWriteFont;
typedef struct IDWriteFontFace IDWriteFontFace;
typedef struct IDWriteFontFile IDWriteFontFile;
typedef struct IDWriteFontFileLoader IDWriteFontFileLoader; 
typedef struct IDWriteFontFileStream IDWriteFontFileStream;

typedef enum DWRITE_FACTORY_TYPE
{
    DWRITE_FACTORY_TYPE_SHARED,
    DWRITE_FACTORY_TYPE_ISOLATED
} DWRITE_FACTORY_TYPE;

typedef enum DWRITE_FONT_WEIGHT
{
    DWRITE_FONT_WEIGHT_THIN = 100,
    DWRITE_FONT_WEIGHT_EXTRA_LIGHT = 200,
    DWRITE_FONT_WEIGHT_ULTRA_LIGHT = 200,
    DWRITE_FONT_WEIGHT_LIGHT = 300,
    DWRITE_FONT_WEIGHT_SEMI_LIGHT = 350,
    DWRITE_FONT_WEIGHT_NORMAL = 400,
    DWRITE_FONT_WEIGHT_REGULAR = 400,
    DWRITE_FONT_WEIGHT_MEDIUM = 500,
    DWRITE_FONT_WEIGHT_DEMI_BOLD = 600,
    DWRITE_FONT_WEIGHT_SEMI_BOLD = 600,
    DWRITE_FONT_WEIGHT_BOLD = 700,
    DWRITE_FONT_WEIGHT_EXTRA_BOLD = 800,
    DWRITE_FONT_WEIGHT_ULTRA_BOLD = 800,
    DWRITE_FONT_WEIGHT_BLACK = 900,
    DWRITE_FONT_WEIGHT_HEAVY = 900,
    DWRITE_FONT_WEIGHT_EXTRA_BLACK = 950,
    DWRITE_FONT_WEIGHT_ULTRA_BLACK = 950
} DWRITE_FONT_WEIGHT;

typedef enum DWRITE_FONT_STRETCH
{
    DWRITE_FONT_STRETCH_UNDEFINED = 0,
    DWRITE_FONT_STRETCH_ULTRA_CONDENSED = 1,
    DWRITE_FONT_STRETCH_EXTRA_CONDENSED = 2,
    DWRITE_FONT_STRETCH_CONDENSED = 3,
    DWRITE_FONT_STRETCH_SEMI_CONDENSED = 4,
    DWRITE_FONT_STRETCH_NORMAL = 5,
    DWRITE_FONT_STRETCH_MEDIUM = 5,
    DWRITE_FONT_STRETCH_SEMI_EXPANDED = 6,
    DWRITE_FONT_STRETCH_EXPANDED = 7,
    DWRITE_FONT_STRETCH_EXTRA_EXPANDED = 8,
    DWRITE_FONT_STRETCH_ULTRA_EXPANDED = 9
} DWRITE_FONT_STRETCH;

typedef enum DWRITE_FONT_STYLE
{
    DWRITE_FONT_STYLE_NORMAL,
    DWRITE_FONT_STYLE_OBLIQUE,
    DWRITE_FONT_STYLE_ITALIC
} DWRITE_FONT_STYLE;

/*******************************
*** Interface IDWriteFactory ***
********************************/

typedef struct IDWriteFactoryVtbl
{
    Com_Base_Decl(IDWriteFactory);
    
    /* IDWriteFactory methods */
    STDMETHOD(GetSystemFontCollection)(IDWriteFactory*, IDWriteFontCollection**, BOOL);
    STDMETHOD(dummy_CreateCustomFontCollection)(void);
    STDMETHOD(dummy_RegisterFontCollectionLoader)(void);
    STDMETHOD(dummy_UnregisterFontCollectionLoader)(void);
    STDMETHOD(dummy_CreateFontFileReference)(void);
    STDMETHOD(dummy_CreateCustomFontFileReference)(void);
    STDMETHOD(dummy_CreateFontFace)(void);
    STDMETHOD(dummy_CreateRenderingParams)(void);
    STDMETHOD(dummy_CreateMonitorRenderingParams)(void);
    STDMETHOD(dummy_CreateCustomRenderingParams)(void);
    STDMETHOD(dummy_RegisterFontFileLoader)(void);
    STDMETHOD(dummy_UnregisterFontFileLoader)(void);
    STDMETHOD(dummy_CreateTextFormat)(void);
    STDMETHOD(dummy_CreateTypography)(void);
    STDMETHOD(dummy_GetGdiInterop)(void);
    STDMETHOD(dummy_CreateTextLayout)(void);
    STDMETHOD(dummy_CreateGdiCompatibleTextLayout)(void);
    STDMETHOD(dummy_CreateEllipsisTrimmingSign)(void);
    STDMETHOD(dummy_CreateTextAnalyzer)(void);
    STDMETHOD(dummy_CreateNumberSubstitution)(void);
    STDMETHOD(dummy_CreateGlyphRunAnalysis)(void);
} IDWriteFactoryVtbl;

typedef struct IDWriteFactory
{
    IDWriteFactoryVtbl* vtbl;
} IDWriteFactory;

#define IDWriteFactory_Release(self) (self)->vtbl->Release(self)
#define IDWriteFactory_GetSystemFontCollection(self, collection, updates) (self)->vtbl->GetSystemFontCollection(self, collection, updates)

/**************************************
*** Interface IDWriteFontCollection ***
***************************************/

typedef struct IDWriteFontCollectionVtbl
{
    Com_Base_Decl(IDWriteFontCollection);
    
    STDMETHOD_(UINT32, GetFontFamilyCount)(IDWriteFontCollection*);
    STDMETHOD(GetFontFamily)(IDWriteFontCollection*, UINT32, IDWriteFontFamily**);
    STDMETHOD(dummy_FindFamilyName)(void);
    STDMETHOD(dummy_GetFontFromFace)(void);
} IDWriteFontCollectionVtbl;

typedef struct IDWriteFontCollection
{
    IDWriteFontCollectionVtbl* vtbl;
} IDWriteFontCollection;

#define IDWriteFontCollection_Release(self) (self)->vtbl->Release(self)
#define IDWriteFontCollection_GetFontFamilyCount(self) (self)->vtbl->GetFontFamilyCount(self)
#define IDWriteFontCollection_GetFontFamily(self, index, family) (self)->vtbl->GetFontFamily(self, index, family)

/**********************************
*** Interface IDWriteFonyFamily ***
***********************************/

typedef struct IDWriteFontFamilyVtbl
{
    Com_Base_Decl(IDWriteFontFamily);
    
    /* IDWriteFontList methods */
    STDMETHOD(dummy_GetFontCollection)(void);
    STDMETHOD(dummy_GetFontCount)(void);
    STDMETHOD(dummy_GetFont)(void);
    
    /* IDWriteFontFamily methods */
    STDMETHOD(GetFamilyNames)(IDWriteFontFamily*, IDWriteLocalizedStrings**);
    STDMETHOD(GetFirstMatchingFont)(IDWriteFontFamily*, DWRITE_FONT_WEIGHT, DWRITE_FONT_STRETCH, DWRITE_FONT_STYLE, IDWriteFont**);
    STDMETHOD(dummy_GetMatchingFonts)(void);
} IDWriteFontFamilyVtbl;

typedef struct IDWriteFontFamily 
{
    IDWriteFontFamilyVtbl* vtbl;
} IDWriteFontFamily;

#define IDWriteFontFamily_Release(self) (self)->vtbl->Release(self)
#define IDWriteFontFamily_GetFamilyNames(self, strings) (self)->vtbl->GetFamilyNames(self, strings)
#define IDWriteFontFamily_GetFirstMatchingFont(self, weight, stretch, style, font) (self)->vtbl->GetFirstMatchingFont(self, weight, stretch, style, font)

/*******************************
 ***  Interface IDWriteFont  ***
 *******************************/

typedef struct IDWriteFontVtbl 
{
    Com_Base_Decl(IDWriteFont);
    
    /* IDWriteFont methods */
    STDMETHOD(GetFontFamily)(IDWriteFont*, IDWriteFontFamily**);
    STDMETHOD(dummy_GetWeight)(void);
    STDMETHOD(dummy_GetStretch)(void);
    STDMETHOD(dummy_GetStyle)(void);
    STDMETHOD(dummy_IsSymbolFont)(void);
    STDMETHOD(dummy_GetFaceNames)(void);
    STDMETHOD(dummy_GetInformationalStrings)(void);
    STDMETHOD(dummy_GetSimulations)(void);
    STDMETHOD(dummy_GetMetrics)(void);
    STDMETHOD(dummy_HasCharacter)(void);
    STDMETHOD(CreateFontFace)(IDWriteFont*, IDWriteFontFace**);
} IDWriteFontVtbl;

typedef struct IDWriteFont 
{
    IDWriteFontVtbl* vtbl;
} IDWriteFont;

#define IDWriteFont_Release(self)             (self)->vtbl->Release(self)
#define IDWriteFont_GetFontFamily(self,a)     (self)->vtbl->GetFontFamily(self,a)
#define IDWriteFont_CreateFontFace(self,a)    (self)->vtbl->CreateFontFace(self,a)

/***********************************
 ***  Interface IDWriteFontFace  ***
 ***********************************/

typedef struct IDWriteFontFaceVtbl
{
    Com_Base_Decl(IDWriteFontFace);
    
    /* IDWriteFontFace methods */
    STDMETHOD(dummy_GetType)(void);
    STDMETHOD(GetFiles)(IDWriteFontFace*, UINT32*, IDWriteFontFile**);
    STDMETHOD(dummy_GetIndex)(void);
    STDMETHOD(dummy_GetSimulations)(void);
    STDMETHOD(dummy_IsSymbolFont)(void);
    STDMETHOD(dummy_GetMetrics)(void);
    STDMETHOD(dummy_GetGlyphCount)(void);
    STDMETHOD(dummy_GetDesignGlyphMetrics)(void);
    STDMETHOD(dummy_GetGlyphIndices)(void);
    STDMETHOD(dummy_TryGetFontTable)(void);
    STDMETHOD(dummy_ReleaseFontTable)(void);
    STDMETHOD(dummy_GetGlyphRunOutline)(void);
    STDMETHOD(dummy_GetRecommendedRenderingMode)(void);
    STDMETHOD(dummy_GetGdiCompatibleMetrics)(void);
    STDMETHOD(dummy_GetGdiCompatibleGlyphMetrics)(void);
} IDWriteFontFaceVtbl;

typedef struct IDWriteFontFace
{
    IDWriteFontFaceVtbl* vtbl;
} IDWriteFontFace;

#define IDWriteFontFace_Release(self)             (self)->vtbl->Release(self)
#define IDWriteFontFace_GetFiles(self,a,b)        (self)->vtbl->GetFiles(self,a,b)

/******************************************
 ***  Interface IDWriteLocalizedStrings  ***
 *******************************************/

typedef struct IDWriteLocalizedStringsVtbl
{
    Com_Base_Decl(IDWriteLocalizedStrings);
    
    /* IDWriteLocalizedStrings methods */
    STDMETHOD(dummy_GetCount)(void);
    STDMETHOD(dummy_FindLocaleName)(void);
    STDMETHOD(dummy_GetLocaleNameLength)(void);
    STDMETHOD(dummy_GetLocaleName)(void);
    STDMETHOD(GetStringLength)(IDWriteLocalizedStrings*, UINT32, UINT32*);
    STDMETHOD(GetString)(IDWriteLocalizedStrings*, UINT32, WCHAR*, UINT32);
} IDWriteLocalizedStringsVtbl;

typedef struct IDWriteLocalizedStrings 
{
    IDWriteLocalizedStringsVtbl* vtbl;
} IDWriteLocalizedStrings;

#define IDWriteLocalizedStrings_Release(self) (self)->vtbl->Release(self)
#define IDWriteLocalizedStrings_GetStringLength(self,a,b) (self)->vtbl->GetStringLength(self,a,b)
#define IDWriteLocalizedStrings_GetString(self,a,b,c) (self)->vtbl->GetString(self,a,b,c)

/********************************
*** Interface IDWriteFontFile ***
*********************************/
typedef struct IDWriteFontFileVtbl
{
    Com_Base_Decl(IDWriteFontFile);
    
    STDMETHOD(GetReferenceKey)(IDWriteFontFile*, void const**, UINT32*);
    STDMETHOD(GetLoader)(IDWriteFontFile*, IDWriteFontFileLoader**);
    STDMETHOD(dummy_Analyze)(void);
} IDWriteFontFileVtbl;

typedef struct IDWriteFontFile
{
    IDWriteFontFileVtbl* vtbl;
} IDWriteFontFile;

#define IDWriteFontFile_Release(self) (self)->vtbl->Release(self)
#define IDWriteFontFile_GetLoader(self,a) (self)->vtbl->GetLoader(self, a)
#define IDWriteFontFile_GetReferenceKey(self,a,b) (self)->vtbl->GetReferenceKey(self,a,b)

/**************************************
*** Interface IDWriteFontFileLoader ***
***************************************/

typedef struct IDWriteFontFileLoaderVtbl
{
    Com_Base_Decl(IDWriteFontFileLoader);
    STDMETHOD(CreateStreamFromKey)(IDWriteFontFileLoader*, void const*, UINT32, IDWriteFontFileStream**);
} IDWriteFontFileLoaderVtbl;

typedef struct IDWriteFontFileLoader
{
    IDWriteFontFileLoaderVtbl* vtbl;
} IDWriteFontFileLoader;

#define IDWriteFontFileLoader_Release(self) (self)->vtbl->Release(self)
#define IDWriteFontFileLoader_CreateStreamFromKey(self,a,b,c) (self)->vtbl->CreateStreamFromKey(self,a,b,c)

/**************************************
*** Interface IDWriteFontFileStream ***
***************************************/

typedef struct IDWriteFontFileStreamVtbl
{
    Com_Base_Decl(IDWriteFontFileStream);
    
    STDMETHOD(ReadFileFragment)(IDWriteFontFileStream*, void const**, UINT64, UINT64, void**);
    STDMETHOD(ReleaseFileFragment)(IDWriteFontFileStream*, void*);
    STDMETHOD(GetFileSize)(IDWriteFontFileStream*, UINT64* FileSize);
    STDMETHOD(dummy_GetLastWriteTime)(void);
} IDWriteFontFileStreamVtbl;

typedef struct IDWriteFontFileStream
{
    IDWriteFontFileStreamVtbl* vtbl;
} IDWriteFontFileStream;

#define IDWriteFontFileStream_Release(self) (self)->vtbl->Release(self)
#define IDWriteFontFileStream_GetFileSize(self,a) (self)->vtbl->GetFileSize(self, a)
#define IDWriteFontFileStream_ReadFileFragment(self,a,b,c,d) (self)->vtbl->ReadFileFragment(self,a,b,c,d)
#define IDWriteFontFileStream_ReleaseFileFragment(self,a) (self)->vtbl->ReleaseFileFragment(self,a)

#endif