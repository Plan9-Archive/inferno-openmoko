@echo off

del emug.exe
C:\MinGW\bin\gcc.exe -E -w -DROOT="\inferno" -DKERNDATE=1666666666 -DEMU=1 -DUNICODE -D_WIN32_WINNT=0x0400 -DOBJTYPE=386 -I. -Iinclude turbo.c 2>l2  >l1.c
C:\MinGW\bin\gcc.exe -o emug.exe l1.c -lgdi32 -lws2_32 -lwinmm -lmpr
exit
