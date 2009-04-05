
@del emut.exe
@C:\bin\tcc\tcc.exe  -o emut.exe -bench -Iinclude -I.. turbo.c -lgdi32 -luser32 -lws2_32 -lwinmm -ladvapi32 -lmpr 2>&1 | perl -pe "s#^/#o:/#"

if not exist emut.exe goto final
emut.exe -r .. wm/wm wm/coffee
rem emut.exe wm/wm charon
:final
exit
