set DEVICE=info
set perlpath=C:\Perl64\bin
set makepath=%ProgramFiles(x86)%\GnuWin32\bin
set EPICS_BASE=C:\SlowControls\EPICS\base-3.14.12.3
set PATH=%EPICS_BASE%\bin\win32-x86;%perlpath%;%makepath%;%PATH%
set EPICS_HOST_ARCH=win32-x86
%EPICS_BASE%\bin\%EPICS_HOST_ARCH%\makeBaseApp.pl -t ioc %DEVICE%
%EPICS_BASE%\bin\%EPICS_HOST_ARCH%\makeBaseApp.pl -a %EPICS_HOST_ARCH% -p %DEVICE% -i -t ioc %DEVICE%
make
copy %DEVICE%Support.dbd %DEVICE%App\src\O.Common
echo include "%DEVICE%Support.dbd" >> "%DEVICE%App\src\O.Common\%DEVICE%Include.dbd"
make
copy %DEVICE%App\src\O.Common\%DEVICE%.dbd ..\%DEVICE%.dbd
copy %DEVICE%App\src\O.win32-x86\%DEVICE%_registerRecordDeviceDriver.cpp ..\%DEVICE%_registerRecordDeviceDriver.cpp
echo Compilation is not required. Ignore any error following the cl command.