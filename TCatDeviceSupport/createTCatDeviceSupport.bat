set DEVICE=tCat
set EPICS_BASE=C:\instrument\apps\EPICS\base\master
set EPICS_HOST_ARCH=win32-x86-debug
set PATH=%EPICS_BASE%\bin\%EPICS_HOST_ARCH%;%PATH%
rem %EPICS_BASE%\bin\%EPICS_HOST_ARCH%\makeBaseApp.pl -t ioc %DEVICE%
rem %EPICS_BASE%\bin\%EPICS_HOST_ARCH%\makeBaseApp.pl -i -t ioc %DEVICE%
copy /y %DEVICE%Support.dbd %DEVICE%App\src
make
copy /y %DEVICE%App\src\O.Common\%DEVICE%.dbd ..\%DEVICE%.dbd
copy /y %DEVICE%App\src\O.%EPICS_HOST_ARCH%\%DEVICE%_registerRecordDeviceDriver.cpp ..\%DEVICE%_registerRecordDeviceDriver.cpp
