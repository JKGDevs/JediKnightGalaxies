:: Moves JKG folder dlls and associated files to correct directory after compile
if not exist ".\JKG" mkdir .\JKG
move /y ".\cgamex86.dll" ".\JKG\cgamex86.dll" 
move /y ".\gamex86.dll" ".\JKG\gamex86.dll" 
move /y ".\uix86.dll" ".\JKG\uix86.dll"


::With the default compiler settings, you want to leave debug info in the root directory with the .exe otherwise it can't find the debug info when the dlls get shipped off to temp land by openJK's dll loader.
::move /y ".\cgamex86.ilk" ".\JKG\cgamex86.ilk"
::move /y ".\cgamex86.pdb" ".\JKG\cgamex86.pdb"
:::move /y ".\gamex86.ilk" ".\JKG\gamex86.ilk"
::move /y ".\gamex86.pdb" ".\JKG\gamex86.pdb"
::move /y ".\uix86.ilk" ".\JKG\uix86.ilk"
::move /y ".\uix86.pdb" ".\JKG\uix86.pdb"

:These are included libraries and don't really need to recompile each time, but this is where they're located.
copy /y "..\..\codemp\libraries\SDL2\bin\x86\SDL2.dll" ".\SDL2.dll"
copy /y "..\..\codemp\OpenAL32.dll" ".\OpenAL32.dll"