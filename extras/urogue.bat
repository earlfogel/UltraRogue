@echo off
rem Run UltraRogue

rem Set Options:
set SROGUEOPTS=cutcorners,difficulty=normal

rem If we have a saved game, resume it, otherwise start a new one.
if exist %APPDATA%\urogue\rogue.save (
	copy /y %APPDATA%\urogue\rogue.save %APPDATA%\urogue\rogue.save.bak >NUL
	urogue.exe %APPDATA%\urogue\rogue.save
) else (
	urogue.exe %*
)
echo.
pause
