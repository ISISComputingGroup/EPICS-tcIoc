#####################################################################
# Install_tcioc
# Author: Daniel Sigg/Maggie Tse
# Date: August 2013
#####################################################################
#
# Powershell script to install the tcIoc binaries
#
# Requires .NET 4.0 or higher
# http://www.microsoft.com/en-us/download/details.aspx?id=30653
# Requires PowerShell 5.0 or higher
# http://www.microsoft.com/en-us/download/details.aspx?id=34595
# Enable script execution by issuing "Set-ExecutionPolicy RemoteSigned"
# once from a powershell command line with administrative priviliges.
# http://technet.microsoft.com/en-us/library/ee176949.aspx
#
# Call from within PowerShell ISE, or 
# Call from command line.
#
#####################################################################
[CmdletBinding()]
param ([switch] $d,
       [switch] $x64)

write "Install tcIoc Binaries"

$version = "2_2"
# Path
$parent = "$PSScriptRoot"
$download = "$parent\Download"
$epicsbase = "..\..\base-3.15.9"
if ($x64) {
    $platform = "x64"
    $epicstarget = "windows-x64"
}
else {
    $platform = "win32"
    $epicstarget = "win32-x86"
}
$destination = "$PSScriptRoot\$platform\Bin"
if ($d) {
    $source = "$parent\$platform\Debug"
    $epicsbin = "$parent\$epicsbase\bin\$epicstarget-debug"
}
else {
    $source = "$parent\$platform\Release"
    $epicsbin = "$parent\$epicsbase\bin\$epicstarget"
}

# create target path if it doesn't exists
if (-not $(test-path $destination)) {
    new-item $destination -type directory > $null
}
# create Download path if it doesn't exists
if (-not $(test-path $download)) {
    new-item $download -type directory > $null
}

write "Epics libraries:   $epicsbin"
write "Install from:      $source"
write "Install to:        $destination"

write "Copying tcIoc.exe"
copy-item $source\tcIoc.exe $destination
#copy-item $source\EpicsDbGen.exe $destination
copy-item $source\tpyinfo.exe $destination
copy-item $source\tcIocSupport.dll $destination
#copy-item $source\EpicsDbLib.lib $destination
#copy-item $source\tcIocSupport.lib $destination
#copy-item $source\tpylib.lib $destination

write "Copying EPICS libraries"
copy-Item $epicsbin\ca.dll $destination
copy-Item $epicsbin\caget.exe $destination
copy-Item $epicsbin\cainfo.exe $destination
copy-Item $epicsbin\camonitor.exe $destination
copy-Item $epicsbin\caput.exe $destination
copy-Item $epicsbin\caRepeater.exe $destination
copy-Item $epicsbin\cas.dll $destination
copy-Item $epicsbin\Com.dll $destination
copy-Item $epicsbin\dbCore.dll $destination
copy-Item $epicsbin\dbRecStd.dll $destination
#copy-Item $epicsbin\gdd.dll $destination
copy-Item $epicsbin\iocLogServer.exe $destination

copy-Item $parent\README.md $destination
copy-Item $parent\COPYING $destination
copy-Item $parent\COPYING-GPL-3 $destination
copy-Item $parent\svn_version.h $destination
copy-Item $parent\tCat.dbd $destination
copy-Item $parent\st.cmd $destination
copy-Item $parent\start.bat $destination


Compress-Archive -Path $destination\* -DestinationPath $download\tcIoc_${platform}_${version}.zip -Force

write-verbose "Done"
