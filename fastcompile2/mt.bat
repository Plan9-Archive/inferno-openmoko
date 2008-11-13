@echo off

del emut.exe
C:\bin\tcc\tcc.exe  -o emut.exe -bench -DROOT="/inferno" -DKERNDATE=1666666666 -DEMU=1 -DUNICODE -D_WIN32_WINNT=0x0400 -DOBJTYPE=386 -Iinclude -Io: turbo.c -lgdi32 -luser32 -lws2_32 -lwinmm -ladvapi32 -lmpr
exit
