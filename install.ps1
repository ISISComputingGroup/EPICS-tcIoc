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
# Requires PowerShell 3.0 or higher
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
param ([switch] $d)

write "Install tcIoc Binaries"

# Path
$parent = "C:\SlowControls"
$destination = "$parent\EPICS\Utilities\Bin"
if ($d) {
    $source = "$parent\EPICS\Utilities\tcIoc\Debug"
    $epicsbin = "$parent\EPICS\base-3.14.12.3\bin\win32-x86-debug"
}
else {
    $source = "$parent\EPICS\Utilities\tcIoc\Release"
    $epicsbin = "$parent\EPICS\base-3.14.12.3\bin\win32-x86"
}

# create target path if it doesn't exists
if (-not $(test-path $destination)) {
    new-item $destination -type directory > $null
}

write "Epics libraries:   $epicsbin"
write "Install from:      $source"
write "Install to:        $destination"

write "Copying tcIoc.exe"
copy-item $source\tcIoc.exe $destination
copy-item $source\EpicsDbGen.exe $destination
copy-item $source\tpyinfo.exe $destination
copy-item $source\tcIocSupport.dll $destination
copy-item $source\EpicsDbLib.lib $destination
copy-item $source\tcIocSupport.lib $destination
copy-item $source\tpylib.lib $destination
copy-item $source\ControlState.lib $destination
copy-item $source\csdinfo.exe $destination

write "Copying EPICS libraries"
copy-Item $epicsbin\asHost.dll $destination
copy-Item $epicsbin\asIoc.dll $destination
copy-Item $epicsbin\ca.dll $destination
copy-Item $epicsbin\caget.exe $destination
copy-Item $epicsbin\cainfo.exe $destination
copy-Item $epicsbin\camonitor.exe $destination
copy-Item $epicsbin\caput.exe $destination
copy-Item $epicsbin\caRepeater.exe $destination
copy-Item $epicsbin\cas.dll $destination
copy-Item $epicsbin\ca_test.exe $destination
copy-Item $epicsbin\Com.dll $destination
copy-Item $epicsbin\dbIoc.dll $destination
copy-Item $epicsbin\dbStaticHost.dll $destination
copy-Item $epicsbin\dbStaticIoc.dll $destination
copy-Item $epicsbin\dbtoolsIoc.dll $destination
copy-Item $epicsbin\gdd.dll $destination
copy-Item $epicsbin\iocLogServer.exe $destination
copy-Item $epicsbin\miscIoc.dll $destination
copy-Item $epicsbin\recIoc.dll $destination
copy-Item $epicsbin\registryIoc.dll $destination
copy-Item $epicsbin\rsrvIoc.dll $destination
copy-Item $epicsbin\softDevIoc.dll $destination
copy-Item $epicsbin\testDevIoc.dll $destination

write-verbose "Done"
