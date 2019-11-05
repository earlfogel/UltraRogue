@echo off
REM Run UltraRogue
REM If we have a saved game, resume it, otherwise start a new one.
if exist %APPDATA%\urogue\rogue.save (
	urogue.exe %APPDATA%\urogue\rogue.save
) else (
	REM add game options to the following line, e.g.: urogue.exe -easy
	urogue.exe
)
echo.
pause
