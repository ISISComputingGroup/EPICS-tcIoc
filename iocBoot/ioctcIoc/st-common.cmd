
tcSetAlias("PLC:TEST:")
tcSetScanRate(10, 5)
#tcLoadRecords ("$(TPY_FILE)", "-ps -devtc -rd -cp -nd")
#tcGenerateList("$(TOP)\test_PLC\TestPLC\TestPLC\TestCode\TestCode.tpy.txt","-l -rn -yi -cp")
tcLoadRecords ("C:\Jenkins\workspace\line_Ticket3991_run_system_tests\dummy_PLC\TestPLC\TestCode\TestCode.tpy", "-eo -devtc")
#tcLoadRecords ("DrivePlc.tpy", "-ea -devtc")
#tcLoadRecords ("DrivePlc.tpy", "-eo -devtc")

iocInit()
