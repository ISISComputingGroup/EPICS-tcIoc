echo this should be run as admin and requires a reboot afterwards.
echo note this may cause all IOCs to error with a "time discontinuity" - to resolve this you will need to run unsettick.bat
bcdedit /set disabledynamictick yes
bcdedit /set useplatformtick yes
