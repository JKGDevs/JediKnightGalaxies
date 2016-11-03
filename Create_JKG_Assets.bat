@ECHO OFF

set PK3ASSETS=zz_JKG_Assets5
set ASSETSFOLDER=JKGalaxies
set GAMEFOLDER=build\GameData\JKG\

7z a -tzip %PK3ASSETS%.pk3 .\%ASSETSFOLDER%\*
move /y %PK3ASSETS%.pk3 %GAMEFOLDER%

pause

