set DEVICE=info
set EPICS_BASE=C:\instrument\apps\epics\base\master
set EPICS_HOST_ARCH=win32-x86
set PATH=%EPICS_BASE%\bin\%EPICS_HOST_ARCH%;%PATH%
%EPICS_BASE%\bin\%EPICS_HOST_ARCH%\makeBaseApp.pl -t ioc %DEVICE%
%EPICS_BASE%\bin\%EPICS_HOST_ARCH%\makeBaseApp.pl -a %EPICS_HOST_ARCH% -p %DEVICE% -i -t ioc %DEVICE%
pause
make
copy %DEVICE%Support.dbd %DEVICE%App\src\O.Common
echo include "%DEVICE%Support.dbd" >> "%DEVICE%App\src\O.Common\%DEVICE%Include.dbd"
make
copy %DEVICE%App\src\O.Common\%DEVICE%.dbd ..\%DEVICE%.dbd
copy %DEVICE%App\src\O.win32-x86\%DEVICE%_registerRecordDeviceDriver.cpp ..\%DEVICE%_registerRecordDeviceDriver.cpp
echo Compilation is not required. Ignore any error following the cl command.