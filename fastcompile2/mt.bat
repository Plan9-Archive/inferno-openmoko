@del emut.exe
@C:\bin\tcc\tcc.exe  -o emut.exe -bench -Iinclude -I.. turbo.c -lgdi32 -luser32 -lws2_32 -lwinmm -ladvapi32 -lmpr 2>&1 | python tccfilter.py
rem c:\perl64\bin\perl -pe "s#^/#o:/#"

if not exist emut.exe goto final

rem emut.exe -g1024x768 -r ..\..\branches\inferno-reference-stackless man disdump
rem emut.exe -g1024x768 -r ..\..\branches\inferno-reference-stackless wm/wm 
rem emut.exe -g1024x768 -r ..\..\branches\inferno-reference-stackless wm/wm charon http://www.yandex.ru/
rem emut.exe -g1024x768 -r ..\..\branches\inferno-reference-stackless wm/wm
rem emut.exe -g1024x768 -r ..\..\branches\inferno-reference-stackless wm/wm wm/coffee

:final
