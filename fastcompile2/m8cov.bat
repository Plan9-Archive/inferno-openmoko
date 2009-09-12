@echo off
echo m8

del emu8.exe

set MS=C:\Program Files\Microsoft Visual Studio 8
if exist "%MS%\VC\bin\cl.exe" goto :msfound
set MS=C:\Program Files (x86)\Microsoft Visual Studio 9.0
if exist "%MS%\VC\bin\cl.exe" goto :msfound
echo Microsoft C not found
exit
:msfound

set SDK=C:\Program Files\Microsoft Visual Studio 8\VC\PlatformSDK
if exist "%SDK%\Include\windows.h" goto :sdkfound
set SDK=C:\Program Files\Microsoft SDKs\Windows\v6.0A
if exist "%SDK%\Include\windows.h" goto :sdkfound
echo Windows SDK not found
exit
:sdkfound

set INCLUDE=%MS%\VC\include;%SDK%\Include
set LIB=%MS%\VC\lib;%SDK%\Lib
PATH=%MS%\Common7\IDE;%MS%\VC\bin;%PATH%

rem debug
cl.exe -RTCsu -Od -Zi -Fm -MTd -WL -FC -W3 -o emu8.exe -DROOT="/inferno" -DKERNDATE=1666666666 -DEMU=1 -DOBJTYPE=386 -Iinclude -I.. turbo.c winmm.lib user32.lib ws2_32.lib gdi32.lib advapi32.lib mpr.lib

rem release
rem cl.exe -DCOVERITY -GLAFy -GS- -Ox -Oi- -MT -WL -FC -W3 -o emu8.exe -DROOT="/inferno" -DKERNDATE=1666666666 -DEMU=1 -DOBJTYPE=386 -Iinclude -I.. turbo.c winmm.lib user32.lib ws2_32.lib gdi32.lib advapi32.lib mpr.lib

pskill cov-internal-ca