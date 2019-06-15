#define _CRT_SECURE_NO_WARNINGS
#include "devTc.h"
#include "ParseTpy.h"
#include "InfoPlc.h"
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
#include <iostream>
#undef _CRT_SECURE_NO_WARNINGS

//int nProcessed = 0;
//clock_t epicsbegin;
//clock_t epicsend;


using namespace std;
using namespace ParseTpy;
using namespace plc;

/** @file devTc.cpp
	Methods for TwinCAT/ADS device support. 
 ************************************************************************/

namespace DevTc {

/* Initialization function that matches an EPICS record with an internal
	record entry
	@param name Name of record (INP/OUT field)
	@param pEpicsRecord Pointer to EPICS record
	@param pRecord Pointer to a base record
	@return true if successful
	@brief Link Record
 ************************************************************************/
//bool linkRecord (std::stringcase name, dbCommon* pEpicsRecord, BaseRecordPtr& pRecord)
//{
//	if (name.empty() ) {
//		printf("Error in inp field for record %s.\n", pEpicsRecord->name);
//		return false;
//	}
//	std::regex rgx("((tc)://((\\b([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.?)+:(8[0-9][0-9]))/)(\\d{1,9})/(\\d{1,9}):(\\d{1,9})");
//
//	std::smatch match;
//
//	if (!std::regex_search(name, match, rgx)) {
//		printf("Name doesn't fit regex for %s, name is %s!!!\n", pEpicsRecord->name, name.c_str());
//		return false;
//	}
//	//for (auto m:match) std::cout << "submatch " << m << '\n';
//
//	// Get PLC name from EPICS name string
//	BasePLCPtr plcMatch = plc::System::get().find (match[1].str().c_str());
//	if (!plcMatch.get()) {
//		printf("PLC not found %s.\n", pEpicsRecord->name);
//		return false;
//	}
//	// Link record object to EPICS record
//	pRecord = plcMatch->find (stringcase(name.c_str()));
//	if (!pRecord.get()) {
//		printf("Record %s.\n", pEpicsRecord->name);
//		return false;
//	}
//	// Create EPICS interface
//	DevTc::EpicsInterface* epics = new DevTc::EpicsInterface (*pRecord);
//
//	// Let EPICS interface know about size of data
//	TcComms::TCatInterface* tcat = dynamic_cast<TcComms::TCatInterface*>(pRecord->get_plcInterface());
//	if (!tcat) {
//		printf("dynamic cast failed, record name is %s.\n", pEpicsRecord->name);
//		return false;
//	}
//
//	epics->set_size(tcat->get_size());
//
//
//	// Link EPICS record to EPICS interface
//	epics->set_pEpicsRecord(pEpicsRecord);
//	// Link EPICS interface to record object
//	pRecord->set_userInterface(epics);
//	return true;
//}

/** Initialization function that matches an EPICS record with an internal
	TwinCAT record entry
	@param pEpicsRecord Pointer to EPICS record
	@param pRecord Pointer to a base record
	@return true if successful
	@brief Link TwinCat Record
 ************************************************************************/
static bool linkTcRecord (dbCommon* pEpicsRecord, plc::BaseRecordPtr& pRecord)
{
	// Let EPICS interface know about size of data
	TcComms::TCatInterface* tcat = dynamic_cast<TcComms::TCatInterface*>(pRecord->get_plcInterface());
	if (!tcat) {
		printf("dynamic cast failed, record name is %s.\n", pEpicsRecord->name);
		return false;
	}

	// Create EPICS interface
	DevTc::EpicsInterface* epics = new (std::nothrow) DevTc::EpicsInterface (*pRecord);
	if (!epics) {
		printf("allocation failed, record name is %s.\n", pEpicsRecord->name);
		return false;
	}
	
	// Link EPICS record to EPICS interface
	epics->set_pEpicsRecord(pEpicsRecord);
	// Link EPICS interface to record object
	pRecord->set_userInterface(epics);

	return true;
}

/** Initialization function that matches an EPICS record with an internal
	info record entry
	@param pEpicsRecord Pointer to EPICS record
	@param pRecord Pointer to a base record
	@return true if successful
	@brief Link Info Record
 ************************************************************************/
static bool linkInfoRecord (dbCommon* pEpicsRecord, plc::BaseRecordPtr& pRecord)
{
	// Let EPICS interface know about size of data
	InfoPlc::InfoInterface* info = dynamic_cast<InfoPlc::InfoInterface*>(pRecord->get_plcInterface());
	if (!info) {
		printf("dynamic cast failed, record name is %s.\n", pEpicsRecord->name);
		return false;
	}

	// Create EPICS interface
	DevTc::EpicsInterface* epics = new (std::nothrow) DevTc::EpicsInterface(*pRecord);
	if (!epics) {
		printf("allocation failed, record name is %s.\n", pEpicsRecord->name);
		return false;
	}

	// Link EPICS record to EPICS interface
	epics->set_pEpicsRecord(pEpicsRecord);
	// Link EPICS interface to record object
	pRecord->set_userInterface(epics);

	return true;
}

/** register_devsup::register_devsup
 ************************************************************************/
register_devsup::register_devsup()
{
	add (tc_regex, linkTcRecord);
	add (info_regex, linkInfoRecord);
}

/** register_devsup::linkRecord
 ************************************************************************/
bool register_devsup::linkRecord (const std::stringcase& inpout, 
	dbCommon* pEpicsRecord, plc::BaseRecordPtr& pRecord)
{
	if (inpout.empty() ) {
		printf ("Error in inp field for record %s.\n", pEpicsRecord->name);
		return false;
	}

	std::smatch match;
	for (auto i : the_register_devsup.tp_list) {
		if (std::regex_search (inpout, match, i.first)) {
			// Get PLC name from EPICS name string
			BasePLCPtr plcMatch = plc::System::get().find (match[1].str().c_str());
			if (!plcMatch.get()) {
				printf ("PLC not found %s.\n", pEpicsRecord->name);
				return false;
			}
			// Link record object to EPICS record
			pRecord = plcMatch->find (inpout);
			if (!pRecord.get()) {
				printf("No PLC record for %s.\n", pEpicsRecord->name);
				return false;
			}
			// Call link function
			return i.second (pEpicsRecord, pRecord);
		}
	}
	printf ("Name doesn't fit regex for %s, link field is %s.\n", 
		pEpicsRecord->name, inpout.c_str());
	return false;
}

/** register_devsup::the_register_devsup
 ************************************************************************/
register_devsup register_devsup::the_register_devsup;


/** EpicsInterface::EpicsInterface
 ************************************************************************/
EpicsInterface::EpicsInterface (plc::BaseRecord& dval)
		: Interface (dval), isPassive (false), isCallback (false),
		pEpicsRecord (nullptr), ioscanpvt (nullptr)
{
	memset (&callbackval, 0, sizeof (callbackval));
}

/** EpicsInterface::get_callbackRequestPending
 ************************************************************************/
bool EpicsInterface::get_callbackRequestPending() const
{
	BaseRecord* rec = get_record();
	return (rec &&rec->UserIsDirty());
}


/** EpicsInterface::push
 ************************************************************************/
bool EpicsInterface::push()
{
	if (isCallback) {
		
		// Generate IO intr request
		if (isPassive) {
//+			callbackRequestPending = true;
			if (callback().priority !=2) {
				return false;
			}
			callbackRequest (&callback());
		}
		else {
			scanIoRequest (get_ioscan());
		}
	}
	return true;
}


/************************************************************************/
/* Create and export device support entry tables (DSETs).
/* These tell EPICS about the functions used for TCat device support.
 ************************************************************************/

// ai record
static devTcDefIn<aival> aival_record_tc_dset;
static devTcDefIn<airval> airval_record_tc_dset;

// aai record
static devTcDefIn<aaival> aaival_record_tc_dset;

// bi record
static devTcDefIn<bival> bival_record_tc_dset;
static devTcDefIn<birval> birval_record_tc_dset;

// longin record
static devTcDefIn<longinval> longinval_record_tc_dset;

// mbbi record
static devTcDefIn<mbbival> mbbival_record_tc_dset;
static devTcDefIn<mbbirval> mbbirval_record_tc_dset;

// mbbiDirect record
static devTcDefIn<mbbiDirectval> mbbiDirectval_record_tc_dset;
static devTcDefIn<mbbiDirectrval> mbbiDirectrval_record_tc_dset;

// stringin record
static devTcDefIn<stringinval> stringinval_record_tc_dset;

// waveform record
static devTcDefWaveformIn<waveformval> waveformval_record_tc_dset;


// ao record
static devTcDefOut<aoval> aoval_record_tc_dset;
static devTcDefOut<aorval> aorval_record_tc_dset;

// aao record
static devTcDefOut<aaoval> aaoval_record_tc_dset;

// bo record
static devTcDefOut<boval> boval_record_tc_dset;
static devTcDefOut<borval> borval_record_tc_dset;

// longout record
static devTcDefOut<longoutval> longoutval_record_tc_dset;

// mbbo record
static devTcDefOut<mbboval> mbboval_record_tc_dset;
static devTcDefOut<mbborval> mbborval_record_tc_dset;

// mbboDirect record
static devTcDefOut<mbboDirectval> mbboDirectval_record_tc_dset;
static devTcDefOut<mbboDirectrval> mbboDirectrval_record_tc_dset;

// stringout record
static devTcDefOut<stringoutval> stringoutval_record_tc_dset;

}

extern "C" {
	using namespace DevTc;

// ai record
epicsExportAddress(dset, aival_record_tc_dset); ///< Record processing entry for ai
epicsExportAddress(dset, airval_record_tc_dset); ///< Record processing entry for raw ai 

// aai record
epicsExportAddress(dset, aaival_record_tc_dset); ///< Record processing entry for aai

// bi record
epicsExportAddress(dset, bival_record_tc_dset);  ///< Record processing entry for bi
epicsExportAddress(dset, birval_record_tc_dset);  ///< Record processing entry for raw bi

// longin record
epicsExportAddress(dset, longinval_record_tc_dset);  ///< Record processing entry for longin

// mbbi record
epicsExportAddress(dset, mbbival_record_tc_dset); ///< Record processing entry for mbbi
epicsExportAddress(dset, mbbirval_record_tc_dset); ///< Record processing entry for raw mbbi

// mbbiDirect record
epicsExportAddress(dset, mbbiDirectval_record_tc_dset); ///< Record processing entry for mbbiDirect
epicsExportAddress(dset, mbbiDirectrval_record_tc_dset); ///< Record processing entry for raw mbbiDirect

// stringin record
epicsExportAddress(dset, stringinval_record_tc_dset); ///< Record processing entry for stringin

// waveform record
epicsExportAddress(dset, waveformval_record_tc_dset); ///< Record processing entry for waveform


// ao record
epicsExportAddress(dset, aoval_record_tc_dset); ///< Record processing entry for ao
epicsExportAddress(dset, aorval_record_tc_dset); ///< Record processing entry for raw ao

// aao record
epicsExportAddress(dset, aaoval_record_tc_dset); ///< Record processing entry for aao

// bo record
epicsExportAddress(dset, boval_record_tc_dset); ///< Record processing entry for bo
epicsExportAddress(dset, borval_record_tc_dset); ///< Record processing entry for raw bo

// longout record
epicsExportAddress(dset, longoutval_record_tc_dset); ///< Record processing entry for longout

// mbbo record
epicsExportAddress(dset, mbboval_record_tc_dset); ///< Record processing entry for mbbo
epicsExportAddress(dset, mbborval_record_tc_dset); ///< Record processing entry for raw mbbo

// mbboDirect record
epicsExportAddress(dset, mbboDirectval_record_tc_dset); ///< Record processing entry for mbboDirect
epicsExportAddress(dset, mbboDirectrval_record_tc_dset); ///< Record processing entry for raw mbboDirect

// stringout record
epicsExportAddress(dset, stringoutval_record_tc_dset); ///< Record processing entry for stringout

}
