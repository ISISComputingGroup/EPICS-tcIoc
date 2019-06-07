set DEVICE=tCat
set perlpath=C:\Straberry\perl\site\bin;C:\Straberry\perl\bin
set makepath=C:\Straberry\c\bin
set EPICS_BASE=C:\SlowControlsMaggie\EPICS\base-3.15.6
set PATH=%EPICS_BASE%\bin\win32-x86;%perlpath%;%makepath%;%PATH%
set EPICS_HOST_ARCH=win32-x86
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
%EPICS_BASE%\bin\%EPICS_HOST_ARCH%\makeBaseApp.pl -t ioc %DEVICE%
%EPICS_BASE%\bin\%EPICS_HOST_ARCH%\makeBaseApp.pl -a %EPICS_HOST_ARCH% -p %DEVICE% -i -t ioc %DEVICE%
gmake
copy %DEVICE%Support.dbd %DEVICE%App\src\O.Common
echo include "%DEVICE%Support.dbd" >> "%DEVICE%App\src\O.Common\%DEVICE%Include.dbd"
gmake
copy %DEVICE%App\src\O.Common\%DEVICE%.dbd ..\%DEVICE%.dbd
copy %DEVICE%App\src\O.win32-x86\%DEVICE%_registerRecordDeviceDriver.cpp ..\%DEVICE%_registerRecordDeviceDriver.cpp
