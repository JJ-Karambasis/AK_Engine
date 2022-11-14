set CurrentPath=%~dp0
set GLMajorVersion=
set GLMinorVersion=

FOR /F ="delims=" %%A IN ('call build/ini.bat /s GPU /i MajorVersion %CurrentPath%..\Configs\build.ini') DO (
	SET GLMajorVersion=%%A
)

FOR /F ="delims=" %%A IN ('call build/ini.bat /s GPU /i MinorVersion %CurrentPath%..\Configs\build.ini') DO (
	SET GLMinorVersion=%%A
)

IF %GLMajorVersion% NEQ 4 (	
	echo GL major version must be 4. Specified %GLMajorVersion%
)

IF %GLMinorVersion% NEQ 6 (	
	echo GL minor version must be 6. Specified %GLMajorVersion%
)

set GPUFlags=-DENGINE_GL_MAJOR_VERSION=%GLMajorVersion% -DENGINE_GL_MINOR_VERSION=%GLMinorVersion% -I%CurrentPath%../Source/ThirdParty/OpenGL-Registry/api -I%CurrentPath%../Source/ThirdParty/EGL-Registry/api
set GPUPath=%CurrentPath%../Source/Runtime/GPU/GL/gl_gpu.c