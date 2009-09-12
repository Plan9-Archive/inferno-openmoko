PATH=C:\prevent-mingw-4.3.1\bin;%PATH%

rm -rf covinferno

rem start /B  python killer.py
cov-build --dir covinferno cmd /c m8cov.bat

cov-analyze.exe --dir covinferno
rem --checker-option check_malloc_wrappers:yes --checker-option enable_deep_read_models:yes

cov-commit-defects.exe  --datadir covdatabase --product inferno --user admin --dir covinferno

rem run viewer
cov-start-gui.exe --datadir covdatabase
start iexplore http://localhost:5467