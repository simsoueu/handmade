cls
@echo off

REM set CommonCompilerFlags=-MTd -nologo -Gm- -GR- -EHa- -Od -Oi -W4 -wd4201 -wd4100 -wd4189 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -FC -Z7
REM set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

REM set winHandmade="..\code\win32_handmade.cpp"
REM set handmade="..\code\handmade.cpp"

REM IF NOT EXIST ..\build mkdir ..\build

REM pushd ..\build
REM del *.pdb  > NUL 2> NUL

REM cl %CommonCompilerFlags% ..\code\handmade.cpp -Fmhandmade.map -LD /link -incremental:no -opt:ref -PDB:handmade_%random%.pdb -EXPORT:GameGetSoundSamples -EXPORT:GameUpdateAndRender
REM cl %CommonCompilerFlags% ..\code\win32_handmade.cpp -Fmwin32_handmade.map /link %CommonLinkerFlags%

REM popd

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

set CommonCompilerFlags= -FC -MTd -nologo -Gm- -GR- -EHa- ^
  -Od -Oi -WX -W4 ^
  -wd4201 -wd4100 -wd4189 -wd4800 -wd4505^
  -DHANDMADE_DEBUG=1 -DHANDMADE_INTERNAL=1 ^
  -DHANDMADE_WIN32=1 ^
  -Z7
set CommonLinkerFlags= -opt:ref ^
    -incremental:no ^
    user32.lib gdi32.lib winmm.lib

set datetimef=%date:~-4%_%date:~3,2%_%date:~0,2%__%time:~0,2%_%time:~3,2%_%time:~6,2%
set datetimef=%datetimef: =_%

del *.pdb > NUL 2> NUL
cl %CommonCompilerFlags% ..\code\handmade.cpp -Fmhandmade.map -LD /link -incremental:no -PDB:handmade_%random%.pdb -EXPORT:GameUpdateAndRender -EXPORT:GameGetSoundSamples
cl %CommonCompilerFlags% ..\code\win32_handmade.cpp -Fmwin32_handmade.map /link %CommonLinkerFlags%
popd