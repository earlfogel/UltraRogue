@echo off
rem Run UltraRogue
rem If we have a saved game, resume it, otherwise start a new one.
if exist %HOMEPATH%\rogue.save (
	urogue.exe %HOMEPATH%\rogue.save
) else (
	urogue.exe
)
echo.
pause
