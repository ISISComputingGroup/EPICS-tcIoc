
tcSetScanRate(10, 5)
tcLoadRecords ("Plc.tpy", "-eo -devtc")
#tcLoadRecords ("DrivePlc.tpy", "-ea -devtc")
#tcLoadRecords ("DrivePlc.tpy", "-eo -devtc")

iocInit()
