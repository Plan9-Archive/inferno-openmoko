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

PATH=C:\prevent-mingw-4.3.1\bin;%PATH%


rem cov-configure.exe --compiler cl.exe

cov-install-gui.exe --password doh123 --datadir covdatabase --product inferno --use-service no --language c
