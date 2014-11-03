# Make the device support for info
#
# escape function
Function escape 
{
    param([string]$str)
    $str -replace '\\', '\\' -replace ' ', '\ '
}
#
# Device name
$devicepath="C:\SlowControls\EPICS\Utilities\tcIoc\InfoDeviceSupport"
$DEVICE="info"
# 
# Windows program directory
if ("${env:ProgramFiles(x86)}" -eq "") {
    ${env:ProgramFiles(x86)}="${env:ProgramFiles}"
}
#
# set up paths
$perlpath="C:\Perl64\bin"
$makepath="${env:ProgramFiles(x86)}\GnuWin32\bin"
$vspath="${env:ProgramFiles(x86)}\Microsoft Visual Studio 11.0\VC"
$sdkpath="${env:ProgramFiles(x86)}\Microsoft SDKs\Windows\v7.1A"
$epicspath="C:\SlowControls\EPICS\base-3.14.12.3"
#
# EPICS architecture
$epicshostarch="win32-x86"
$epicstargetarch="win32-x86-debug"
#
# change to EPICS base
cd $epicspath
#
# set environment
$env:Path="${env:Path};$perlpath;$makepath;$epicspath\bin\$epicshostarch;$vspath\bin;$sdkpath\Bin;$vspath\..\Common7\IDE"
$env:EPICS_HOST_ARCH="$epicshostarch"
#
# Include path for rc command
$env:INCLUDE="$vspath\include;$sdkpath\Include"
#
# Additional C options
$env:CFLAGS="-I$(escape $vspath)\\include -I$(escape $sdkpath)\\Include"
#
# Additional C++ options
$env:CPPFLAGS="-I$(escape $vspath)\\include -I$(escape $sdkpath)\\Include"
#
# Additional linker flags
$env:LIB="$vspath\Lib;$sdkpath\Lib"
#
# go to device dir
cd $devicepath
#
# delete old files
rm -r bin -ErrorAction SilentlyContinue 
rm -r configure -ErrorAction SilentlyContinue 
rm -r dbd -ErrorAction SilentlyContinue 
rm -r ${DEVICE}App -ErrorAction SilentlyContinue 
rm -r iocBoot -ErrorAction SilentlyContinue 
rm Makefile -ErrorAction SilentlyContinue 
#
# call makeBasApp
. "$epicspath\bin\$epicshostarch\makeBaseApp.pl" -t ioc $DEVICE
. "$epicspath\bin\$epicshostarch\makeBaseApp.pl" -a $epicshostarch -p $DEVICE -i -t ioc $DEVICE
sleep 5
#
# make base app
make
copy ${DEVICE}Support.dbd ${DEVICE}App\src\O.Common\
add-content "${DEVICE}App\src\O.Common\${DEVICE}Include.dbd" "include ""${DEVICE}Support.dbd"""
make -i 
#
# copy generated device support files back to main dir
copy ${DEVICE}App\src\O.Common\$DEVICE.dbd ..\$DEVICE.dbd
copy ${DEVICE}App\src\O.win32-x86\${DEVICE}_registerRecordDeviceDriver.cpp ..\${DEVICE}_registerRecordDeviceDriver.cpp