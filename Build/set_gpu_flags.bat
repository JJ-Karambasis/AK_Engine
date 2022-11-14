set CurrentPath=%~dp0

SET GPUApi=
FOR /F ="delims=" %%A IN ('call build/ini.bat /s GPU /i Api %CurrentPath%..\Configs\build.ini') DO (
	SET GPUApi=%%A
)

IF %GPUApi% == GL (
	call %CurrentPath%set_gl_flags.bat
)