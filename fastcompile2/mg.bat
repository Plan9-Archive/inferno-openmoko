rem @echo off

del emug.exe
rem C:\MinGW\bin\gcc.exe -E  -DROOT="\inferno" -DKERNDATE=1666666666 -DEMU=1 -DUNICODE -D_WIN32_WINNT=0x0400 -DOBJTYPE=386 -I. -Iinclude turbo.c 2>l2  >l1.c
rem rem set LARCH_PATH=O:\inferno\fastcompile2\ctool\splint-3.0.1.6\lib
rem rem O:\inferno\fastcompile2\ctool\splint-3.0.1.6\bin\splint.exe l1.c >l1.lint
rem exit
rem C:\MinGW\bin\gcc.exe -o emug.exe l1.c -lgdi32 -lws2_32 -lwinmm -lmpr

rem -w
C:\bin\MinGW\bin\gcc.exe turbo.c -o emug.exe  -I.. -Iinclude -lgdi32 -lws2_32 -lwinmm -lmpr 2>&1  | python tccfilter.py
rem | perl -pe "s#^/#o:/#"

if not exist emug.exe goto final
rem emug.exe -r .. wm/wm wm/coffee
:final
exit
