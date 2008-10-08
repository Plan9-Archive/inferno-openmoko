rem build native inferno

del O:\Inferno\Inferno\386\pbs
del O:\Inferno\Inferno\386\9load
del O:\Inferno\Inferno\386\disk
del O:\Inferno\Inferno\386\bin\ipc
del O:\Inferno\Inferno\386\bin\ipc.gz
del O:\Inferno\os\boot\pc\pbs
del O:\Inferno\os\boot\pc\9load

cd O:\Inferno\os\boot\pc
mk pbs.install 9load.install


cd O:\Inferno\os\pc
mk install

cd O:\Inferno\Inferno\386\bin
gzip ipc

cd O:\Inferno
O:\Inferno\Nt\386\bin\emu.exe -r O:\Inferno disk/format -b /Inferno/386/pbs -df /Inferno/386/disk /Inferno/386/9load /os/pc/plan9.ini /Inferno/386/bin/ipc.gz


o:\inferno\qemu\386\qemu.exe -L o:\inferno\qemu\386 -m 128 -soundhw all -localtime -M isapc -fda O:\Inferno\Inferno\386\disk
rem o:\inferno\qemu\386\qemu.exe -L o:\inferno\qemu\386 -m 128 -soundhw all -localtime -M isapc -fda O:\work\openmoko\inferno-images\inferno1440.img

rem start o:\inferno\qemu\386\qemu.exe -L o:\inferno\qemu\386 -m 128 -soundhw all -localtime -M isapc -fda O:\Inferno\Inferno\386\disk
rem start o:\inferno\qemu\386\qemu.exe -L o:\inferno\qemu\386 -m 128 -soundhw all -localtime -M isapc -fda O:\Inferno-vn\Inferno\386\disk
