dbLoadDatabase("./tCat.dbd",0,0)
tCat_registerRecordDeviceDriver(pdbbase)

tcSetScanRate(10, 5)
tcLoadRecords ("DrivePlc.tpy", "-ea -devtc")
#tcLoadRecords ("DrivePlc.tpy", "-eo -devtc")

iocInit()
