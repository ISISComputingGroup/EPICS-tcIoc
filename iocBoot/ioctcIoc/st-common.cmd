## Ams netid of current computer we are running on
epicsEnvSet("TCIOC_SETLOCAL", "130.246.39.141.1.1")

## IP address of computer/plc specified in tpy file that we 
## wish to access. An ADS route will get added using this address
epicsEnvSet("TCIOC_ADDROUTE", "130.246.53.239")

## print some debug information
epicsEnvSet("TCIOC_DEBUG","1")
epicsEnvSet("TCIOC_TCDEBUG", "1")

tcSetScanRate(150, 5)

## load tpy file. It is very important this TPY file is 
## the latest one from the PLC visual studio build. 
## If not, you may be able to connect ut any reads/writes
## will just show 0. Even if the PLC code has not changed, if
## visual studio builds a new TPY file you need to use this here
## we also use -eo here so we only export PLC variables
## marked with IOC export directies. Use -ea to export all
## variables
tcLoadRecords ("TestCode.tpy", "-eo -devtc -p MYPREFIX:")

#tcLoadRecords ("DrivePlc.tpy", "-ea -devtc")
#tcLoadRecords ("DrivePlc.tpy", "-eo -devtc")

iocInit()
