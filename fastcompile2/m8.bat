@echo off
echo m8
rem cd O:\Inferno\fastcompile2
rem cd D:\work\inferno-openmoko\trunk\

del emu8.exe
del l1.obj

set MS=C:\Program Files (x86)\Microsoft Visual Studio 9.0

rem set SDK=C:\Program Files\Microsoft Visual Studio 8\VC\PlatformSDK
set SDK=C:\Program Files\Microsoft SDKs\Windows\v6.0A
set INCLUDE=%MS%\VC\include;%SDK%\Include
set LIB=%MS%\VC\lib;%SDK%\Lib
set PATH=%MS%\Common7\IDE;%MS%\VC\bin

rem cl.exe -analyze 1.c
rem exit

rem cl.exe -EP -DROOT="/inferno" -DKERNDATE=1666666666 -DEMU=1 -DUNICODE -D_WIN32_WINNT=0x0400 -DOBJTYPE=386 -I. turbo.c >l2 >l1.c
rem cl.exe -c l1.c >l22
rem link.exe l1.obj winmm.lib user32.lib ws2_32.lib gdi32.lib advapi32.lib mpr.lib
rem cl /analyze >l
rem cl /? >l
rem -GAFy -GS- -Ox -Oi-
rem -analyze



rem debug
rem -RTCc causes wm not working
rem  -showIncludes
rem  -analyze
cl.exe -RTCsu -Od -Zi -Fm -MTd -WL -FC -W3 -o emu8.exe -DROOT="/inferno" -DKERNDATE=1666666666 -DEMU=1 -DOBJTYPE=386 -Iinclude -I.. turbo.c winmm.lib user32.lib ws2_32.lib gdi32.lib advapi32.lib mpr.lib
goto probe

rem release
cl.exe -GLAFy -GS- -Ox -Oi- -MT -WL -FC -W3 -o emu8.exe -DROOT="/inferno" -DKERNDATE=1666666666 -DEMU=1 -DOBJTYPE=386 -Iinclude -I.. turbo.c winmm.lib user32.lib ws2_32.lib gdi32.lib advapi32.lib mpr.lib
goto probe



:probe

if not exist emu8.exe goto final
emu8.exe -g1024x768 -r ..\..\branches\inferno-reference-stackless man disdump 
rem emu8.exe -g1024x768 -r ..\..\branches\inferno-reference-stackless wm/wm charon http://www.yandex.ru/
rem emut.exe -r .. wm/wm
rem charon
rem emut.exe -r .. wm/wm wm/coffee
rem emut.exe -r .. wm/wm charon
:final
exit
