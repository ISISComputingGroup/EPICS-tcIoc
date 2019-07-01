set EPICS_BASE=C:\SlowControls\EPICS\base-3.14.12.3
set PATH=%EPICS_BASE%\bin\win32-x86;%PATH%
set EPICS_HOST_ARCH=win32-x86

cd %~dp0
tcIoc.exe st.cmd
