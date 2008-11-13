@echo off

del emu5.exe
del l1.obj

set MS=C:\Program Files\Microsoft Visual Studio 8

set SDK=C:\Program Files\Microsoft Visual Studio 8\VC\PlatformSDK
set INCLUDE=%MS%\VC\include;%SDK%\Include
set LIB=%MS%\VC\lib;%SDK%\Lib
set PATH=%MS%\Common7\IDE;%MS%\VC\bin

rem cl.exe -EP -DROOT="/inferno" -DKERNDATE=1666666666 -DEMU=1 -DUNICODE -D_WIN32_WINNT=0x0400 -DOBJTYPE=386 -I. turbo.c >l2 >l1.c
rem cl.exe -c l1.c >l22
rem link.exe l1.obj winmm.lib user32.lib ws2_32.lib gdi32.lib advapi32.lib mpr.lib
rem cl /analyze >l
cl /? >l
rem -GAFy -GS- -Ox -Oi-

cl.exe  -Zi -MT -WL -FC -W3 -o emu5.exe -DROOT="/inferno" -DKERNDATE=1666666666 -DEMU=1 -DUNICODE -D_WIN32_WINNT=0x0400 -DOBJTYPE=386 -Iinclude turbo.c winmm.lib user32.lib ws2_32.lib gdi32.lib advapi32.lib mpr.lib
exit
