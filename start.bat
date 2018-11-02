setlocal
set EPICS_BASE=C:\Instrument\Apps\EPICS\base\master
set EPICS_HOST_ARCH=win32-x86-debug
set "PATH=%EPICS_BASE%\bin\%EPICS_HOST_ARCH%;%PATH%"

cd %~dp0
rem Release\tcIoc.exe st-isis.cmd
Debug\tcIoc.exe 
