PATH=C:\prevent-mingw-4.1.0\bin;%PATH%


cov-build --dir covinferno cmd /c m8.bat

cov-analyze.exe --dir covinferno

cov-commit-defects.exe  --datadir covdatabase --product inferno --user admin --dir covinferno

rem run viewer
rem cov-start-gui.exe --datadir covdatabase
rem start iexplore http://localhost:5467