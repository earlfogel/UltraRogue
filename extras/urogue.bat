@echo off
rem Run UltraRogue

rem Set Options:
rem set SROGUEOPTS=cutcorners,difficulty=easy

rem If we have a saved game, resume it, otherwise start a new one.
if exist %APPDATA%\urogue\rogue.save (
	urogue.exe %APPDATA%\urogue\rogue.save
) else (
	urogue.exe %*
)
echo.
pause
