set CurrentPath=%~dp0

set FreeTypeIncludePath=-I%CurrentPath%../Source/ThirdParty/freetype/include
set FreeTypePath=%CurrentPath%../Source/ThirdParty/freetype/src
set FreetypeCFiles=%FreeTypePath%\base\ftsystem.c ^
	%FreeTypePath%\base\ftinit.c ^
	%FreeTypePath%\base\ftdebug.c ^
	%FreeTypePath%\base\ftbase.c ^
	%FreeTypePath%\truetype\truetype.c ^
	%FreeTypePath%\sdf\sdf.c ^
	%FreeTypePath%\base\ftbitmap.c ^
	%FreeTypePath%\autofit\autofit.c ^
	%FreeTypePath%\type1\type1.c ^
	%FreeTypePath%\cff\cff.c ^
	%FreeTypePath%\cid\type1cid.c ^
	%FreeTypePath%\pfr\pfr.c ^
	%FreeTypePath%\type42\type42.c ^
	%FreeTypePath%\winfonts\winfnt.c ^
	%FreeTypePath%\pcf\pcf.c ^
	%FreeTypePath%\bdf\bdf.c ^
	%FreeTypePath%\psaux\psaux.c ^
	%FreeTypePath%\psnames\psnames.c ^
	%FreeTypePath%\sfnt\sfnt.c ^
	%FreeTypePath%\smooth\smooth.c ^
	%FreeTypePath%\raster\raster.c ^
	%FreeTypePath%\gzip\ftgzip.c ^
	%FreeTypePath%\lzw\ftlzw.c ^
	%FreeTypePath%\pshinter\pshinter.c ^
	%FreeTypePath%\base\ftsynth.c ^
	%FreeTypePath%\svg\ftsvg.c