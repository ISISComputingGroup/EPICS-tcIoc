#define _CRT_SECURE_NO_WARNINGS
#include "devInfo.h"
#include "ParseTpy.h"
#include "infoPlc.h"
#include "errlog.h"
#undef va_start
#undef va_end
#include "dbAccess.h"
#include "dbCommon.h"
#include "dbEvent.h"
#include "recGbl.h"
#include "recSup.h"
#include "epicsExport.h"
#include "aitConvert.h"
#include <regex>
#include <iostream>
#undef _CRT_SECURE_NO_WARNINGS


using namespace std;
using namespace ParseTpy;
using namespace plc;
using namespace DevTc;

namespace DevInfo {

/* linkTcRecord
 ************************************************************************/
bool linkInfoRecord (dbCommon* pEpicsRecord, BaseRecordPtr& pRecord)
{
	// Create EPICS interface
	DevTc::EpicsInterface* epics = new (std::nothrow) DevTc::EpicsInterface (*pRecord);
	if (!epics) {
		printf("new failed, record name is %s.\n", pEpicsRecord->name);
		return false;
	}

	// Link EPICS record to EPICS interface
	epics->set_pEpicsRecord(pEpicsRecord);
	// Link EPICS interface to record object
	pRecord->set_userInterface(epics);
	return true;
}

/* register_info_devsup::the_register_info_devsup
 ************************************************************************/
register_info_devsup register_info_devsup::the_register_info_devsup;

/************************************************************************/
/* Create and export device support entry tables (DSETs).
/* These tell EPICS about the functions used for Info device support.
 ************************************************************************/

// ai record
static devTcDefIn<aival> aival_record_info_dset;
// bi record
static devTcDefIn<bival> bival_record_info_dset;
// longin record
static devTcDefIn<longinval> longinval_record_info_dset;
// mbbi record
static devTcDefIn<mbbival> mbbival_record_info_dset;
// stringin record
static devTcDefIn<stringinval> stringinval_record_info_dset;

// ao record
static devTcDefOut<aoval> aoval_record_info_dset;
// bo record
static devTcDefOut<boval> boval_record_info_dset;
// longout record
static devTcDefOut<longoutval> longoutval_record_info_dset;
// mbbo record
static devTcDefOut<mbboval> mbboval_record_info_dset;
// stringout record
static devTcDefOut<stringoutval> stringoutval_record_info_dset;

}

extern "C" {
	using namespace DevInfo;

// ai record
epicsExportAddress(dset, aival_record_info_dset);
// bi record
epicsExportAddress(dset, bival_record_info_dset);
// longin record
epicsExportAddress(dset, longinval_record_info_dset);
// mbbi record
epicsExportAddress(dset, mbbival_record_info_dset);
// stringin record
epicsExportAddress(dset, stringinval_record_info_dset);

// ao record
epicsExportAddress(dset, aoval_record_info_dset);
// bo record
epicsExportAddress(dset, boval_record_info_dset);
// longout record
epicsExportAddress(dset, longoutval_record_info_dset);
// mbbo record
epicsExportAddress(dset, mbboval_record_info_dset);
// stringout record
epicsExportAddress(dset, stringoutval_record_info_dset);

}
