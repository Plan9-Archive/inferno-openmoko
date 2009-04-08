PATH=C:\prevent-mingw-4.1.0\bin;%PATH%


cov-build --dir covinferno cmd /c m8cov.bat

cov-analyze.exe --dir covinferno

cov-commit-defects.exe  --datadir covdatabase --product inferno --user admin --dir covinferno

rem run viewer
cov-start-gui.exe --datadir covdatabase
start iexplore http://localhost:5467