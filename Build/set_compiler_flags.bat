set CurrentPath=%~dp0

SET CompileFreetype=False
FOR /F ="delims=" %%A IN ('call build/ini.bat /s CompilerSettings /i CompileFreetype %CurrentPath%..\Configs\build.ini') DO (
	SET CompileFreetype=%%A
)

IF NOT %CompileFreetype% == False (
	IF NOT %CompileFreetype% == True (
		echo Unknown CompileFreetype value %CompileFreetype%. Must be True or False
		EXIT /b 1
	)
)

SET Optimized=False
FOR /F ="delims=" %%A IN ('call build/ini.bat /s CompilerSettings /i Optimized %CurrentPath%..\Configs\build.ini') DO (
	SET Optimized=%%A
)

IF %Optimized% == False (
	set OptimizedFlags=-DDEBUG_BUILD	
) ELSE IF %Optimized% == True (
	set OptimizedFlags=
) ELSE (
	echo Unknown Optimized flag %Optimized%. Must be True or False
	EXIT /b 1
)

SET EnableAsserts=False
FOR /F ="delims=" %%A IN ('call build/ini.bat /s CompilerSettings /i EnableAsserts %CurrentPath%..\Configs\build.ini') DO (
	SET EnableAsserts=%%A
)

IF %EnableAsserts% == True (
	set AssertFlags=-DENABLE_ASSERTS	
) ELSE IF %EnableAsserts% == False (
	set AssertFlags=	
) ELSE (
	echo Unknown EnableAssert flag %EnableAsserts%. Must be True or False
	EXIT /b 1
)

SET Compile32Bit=False
FOR /F ="delims=" %%A IN ('call build/ini.bat /s CompilerSettings /i Compile32Bit %CurrentPath%..\Configs\build.ini') DO (
	SET Compile32Bit=%%A
)

set BatchfileVersion=
IF %Compile32Bit% == True (
	set BitnessFlag=-DBITNESS_32	
) ELSE IF %Compile32Bit% == False (
	set BitnessFlag=	
) ELSE (
	echo Unknown Compile32Bit flag %Compile32Bit%. Must be True or False
	EXIT /b 1
)


SET InstallPath=
FOR /F ="delims=" %%A IN ('call build/ini.bat /s Installation /i InstallPath %CurrentPath%..\Configs\build.ini') DO (
	SET InstallPath=%%A
)

IF NOT DEFINED InstallPath (	
	EXIT /b 1
)

IF NOT EXIST "%InstallPath%" (
	ECHO Install path directory %InstallPath% does not exist
	EXIT /b 1
)

xcopy "%CurrentPath%..\Data\" "%InstallPath%\Data\" /E /Y /D

SET IncludePaths=-I%CurrentPath%../Source/Runtime -I%CurrentPath%../Source/Editor -I%CurrentPath%../Source/ThirdParty
set CommonFlags=-DOS_WIN32 -DEDITOR_BUILD
set Libs=advapi32.lib