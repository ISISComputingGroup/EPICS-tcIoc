@echo OFF
REM automatically generated by make using RULES.ioc.ISIS
setlocal
set MYDIR=%~dp0
set MYDIRROOT=%MYDIR:~0,-1%
REM Restrict IOC channel access requests to localhost - we use a gateway for all external access 
set OLDEPICS_CAS_INTF_ADDR_LIST=%EPICS_CAS_INTF_ADDR_LIST%
set OLDEPICS_CAS_BEACON_ADDR_LIST=%EPICS_CAS_BEACON_ADDR_LIST%
set EPICS_CAS_INTF_ADDR_LIST=127.0.0.1
set EPICS_CAS_BEACON_ADDR_LIST=127.255.255.255
set EPICS_CAS_AUTO_BEACON_ADDR_LIST=NO
for /F %%I in ( "%MYDIRROOT%" ) do set IOCDIRNAME=%%~nI
set IOCEXE=%IOCDIRNAME:~3%.exe
set "OLDPATH=%PATH%"
call \config_env_base.bat
set "PREPATH=%PATH%"
call \utils_win32\master\bin\shorten_path.bat PREPATH
set "PATH="
call %MYDIR%dllPath.bat
call \utils_win32\master\bin\shorten_path.bat PATH
set "PATH=%PATH%;%PREPATH%"
if "%1" == "" (
   set "CMDST=st.cmd"
) else (
   set "CMDST=%1"
)
%MYDIR%..\..\bin\win32-x86\%IOCEXE% %CMDST%
set "PATH=%OLDPATH%"
set EPICS_CAS_INTF_ADDR_LIST=%OLDEPICS_CAS_INTF_ADDR_LIST%
set EPICS_CAS_BEACON_ADDR_LIST=%OLDEPICS_CAS_BEACON_ADDR_LIST%
endlocal