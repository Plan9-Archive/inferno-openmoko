@del emut.exe
@C:\bin\tcc\tcc.exe  -o emut.exe -bench -Iinclude -I.. turbo.c -lgdi32 -luser32 -lws2_32 -lwinmm -ladvapi32 -lmpr 2>&1 
rem | perl -pe "s#^/#o:/#"

if not exist emut.exe goto final
emut.exe -g1024x768 -r ..\..\branches\inferno-reference-stackless wm/wm 
rem emut.exe -r .. wm/wm
rem charon
rem emut.exe -r .. wm/wm wm/coffee
rem emut.exe -r .. wm/wm charon
:final
