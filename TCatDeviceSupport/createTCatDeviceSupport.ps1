# Make the device support for info
#
# Device name
$devicepath="C:\SlowControlsMaggie\EPICS\Utilities\tcIoc\TCatDeviceSupport"
$DEVICE="tCat"
$DEVICE2="info"
# 
# Windows program directory
if ("${env:ProgramFiles(x86)}" -eq "") {
    ${env:ProgramFiles(x86)}="${env:ProgramFiles}"
}
#
# set up paths
$perlpath="C:\Straberry\perl\site\bin;C:\Straberry\perl\bin"
$makepath="C:\Straberry\c\bin"
$vspath="${env:ProgramFiles(x86)}\Microsoft Visual Studio\2017\Community\VC"
#$sdkpath="${env:ProgramFiles(x86)}\Microsoft SDKs\Windows\v7.1A"
$epicspath="C:\SlowControlsMaggie\EPICS\base-3.15.6"
#
# EPICS architecture
$epicshostarch="win32-x86"
$epicstargetarch="win32-x86-debug"
#
# change to EPICS base
cd $epicspath
#
# set environment
$env:Path="${env:Path};$perlpath;$makepath;$epicspath\bin\$epicshostarch"
$env:EPICS_HOST_ARCH="$epicshostarch"
#
# Setup compiler
Invoke-BatchFile "${vspath}\Auxiliary\Build\vcvarsall.bat" x86
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
gmake
copy ${DEVICE}Support.dbd ${DEVICE}App\src\O.Common\
copy ${DEVICE2}Support.dbd ${DEVICE}App\src\O.Common\
#echo include "${DEVICE}Support.dbd" >> "${DEVICE}App\src\O.Common\${DEVICE}Include.dbd"
add-content "${DEVICE}App\src\Makefile" "tCat_DBD += ${DEVICE}Support.dbd ${DEVICE2}Support.dbd"
rm ${DEVICE}App\src\O.Common\${DEVICE}.dbd
gmake
#
# copy generated device support files back to main dir
copy ${DEVICE}App\src\O.Common\$DEVICE.dbd ..\$DEVICE.dbd
copy ${DEVICE}App\src\O.win32-x86\${DEVICE}_registerRecordDeviceDriver.cpp ..\${DEVICE}_registerRecordDeviceDriver.cpp
#
echo "Ignore unresolved external symbols!"