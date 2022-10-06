#include "devTc.h"
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
#if EPICS_VERSION < 7
#include "epicsRingPointer.h"
#else
#include "callback.h"
#endif
#include <iostream>
#if defined(_MSC_VER) && (EPICS_VERSION < 7)
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#endif
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

/** Initialization function that matches an EPICS record with an internal
	TwinCAT record entry
	@param pEpicsRecord Pointer to EPICS record
	@param pRecord Pointer to a base record
	@return true if successful
	@brief Link TwinCat Record
 ************************************************************************/
static bool linkTcRecord (dbCommon* pEpicsRecord, plc::BaseRecordPtr& pRecord) noexcept
{
	// Let EPICS interface know about size of data
	const TcComms::TCatInterface* const tcat = dynamic_cast<TcComms::TCatInterface*>(pRecord->get_plcInterface());
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
static bool linkInfoRecord (dbCommon* pEpicsRecord, plc::BaseRecordPtr& pRecord) noexcept
{
	// Let EPICS interface know about size of data
	const InfoPlc::InfoInterface* const info = dynamic_cast<InfoPlc::InfoInterface*>(pRecord->get_plcInterface());
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
register_devsup::register_devsup() noexcept
{
	add (tc_regex, linkTcRecord);
	add (info_regex, linkInfoRecord);
}

/** register_devsup::linkRecord
 ************************************************************************/
bool register_devsup::linkRecord (const std::stringcase& inpout, 
	dbCommon* pEpicsRecord, plc::BaseRecordPtr& pRecord) noexcept
{
	if (inpout.empty() ) {
		printf ("Error in inp field for record %s.\n", pEpicsRecord->name);
		return false;
	}

	try {
		std::smatch match;
		for (const auto& i : the_register_devsup.tp_list) {
			if (std::regex_search(inpout, match, i.first)) {
				// Get PLC name from EPICS name string
				BasePLCPtr plcMatch = plc::System::get().find(match[1].str().c_str());
				if (!plcMatch.get()) {
					printf("PLC not found %s.\n", pEpicsRecord->name);
					return false;
				}
				// Link record object to EPICS record
				pRecord = plcMatch->find(inpout);
				if (!pRecord.get()) {
					printf("No PLC record for %s.\n", pEpicsRecord->name);
					return false;
				}
				// Call link function
				return i.second(pEpicsRecord, pRecord);
			}
		}
	}
	catch (...) {}
	printf ("Name doesn't fit regex for %s, link field is %s.\n", 
		pEpicsRecord->name, inpout.c_str());
	return false;
}

/** register_devsup::the_register_devsup
 ************************************************************************/
register_devsup register_devsup::the_register_devsup;


/* EpicsInterface::get_callbackRequestPending
 ************************************************************************/
bool EpicsInterface::get_callbackRequestPending() const noexcept
{
	return (get_record().UserIsDirty());
}

/* Callback complete_io_scan
 ************************************************************************/
void complete_io_scan (EpicsInterface* epics, IOSCANPVT ioscan, int prio) noexcept
{
	epics->ioscan_reset(prio);
}

/* EpicsInterface::ioscan_reset
 ************************************************************************/
void EpicsInterface::ioscan_reset (int bitnum) noexcept
{
	try {
		std::lock_guard guard(ioscanmux);
		std::atomic_fetch_and(&ioscan_inuse, ~(1 << bitnum));
	}
	catch (...) { }
}

/* EpicsInterface::push
 ************************************************************************/
bool EpicsInterface::push() noexcept
{
	static int skip = 0;
	static int proc = 0;

	if (isCallback) {
		
		// Generate IO intr request
		if (isPassive) {
			if (callback().priority != priorityHigh) {
				return false;
			}
			if (callbackRequest(&callback()) != 0) {
				return false;
			}
//+			callbackRequestPending = true;
		}
		else {
			try {
				std::lock_guard guard(ioscanmux);
				if (ioscan_inuse.load() == 0) ++proc; else ++skip;
				std::atomic_store(&ioscan_inuse, scanIoRequest(get_ioscan()));
			}
			catch (...) {}

			/*
			if (ioscan_inuse.load() == 0) {
				std::lock_guard guard(ioscanmux);
				std::atomic_store(&ioscan_inuse, scanIoRequest(get_ioscan()));
				++proc;
				//printf("Processing %s %4.1f %%\n", record.get_name().c_str(), 100.0*(double)skip / (double)(skip + proc));
			}
			else {
				// skip  ioscan due to overload
				++skip;
				//std::lock_guard guard(ioscanmux);
				//std::atomic_store(&ioscan_inuse, 0);
				//if (skip % 100 == 0) {
				//	printf("Skipping %s %4.1f %%\n", record.get_name().c_str(), 100.0*(double)skip / (double)(skip + proc));
				//}
			}
			*/
		}

	}
	return true;
}

#if defined(_MSC_VER) && (EPICS_VERSION < 7)
/* load_callback_queue variable
 ************************************************************************/
 /// @cond Doxygen_Suppress
const char* const callback_queue_library = "dbCore.dll";
const char* const callback_queue_symbol = "tcat_callbackQueue";
using callback_queue_func = epicsRingPointerId(__cdecl *)(int);
static std::atomic<bool> callback_queue_init = false;
static std::mutex callback_queue_mux;
static epicsRingPointerId callback_queue[3] = { nullptr, nullptr, nullptr };
static int callback_queue_max[3] = { 0, 0, 0 };
#else
static std::mutex callback_queue_mux;
callbackQueueStats callback_status{};
#endif

#if defined(_MSC_VER) && (EPICS_VERSION < 7)
/* load_callback_queue_func
   Looking for tcat_queuePriorityHigh in dbCore.dll
 ************************************************************************/
static bool load_callback_queue_func() noexcept
{
	bool RunTimeLinkSuccess = false;
	// Use dynamic DLL linking in case of unpatched EPICS base
	HINSTANCE hinstLib{};
	callback_queue_func func = nullptr;
	// Get a handle to the DLL module.
	hinstLib = LoadLibrary (callback_queue_library);
	// If the handle is valid, try to get the function address.
	if (hinstLib != nullptr) {
		func = (callback_queue_func)GetProcAddress (hinstLib, callback_queue_symbol);
		// If the function address is valid, call the function.
		if (func != nullptr) {
			RunTimeLinkSuccess = true;
			callback_queue[priorityLow] = func(priorityLow);
			callback_queue[priorityMedium] = func(priorityMedium);
			callback_queue[priorityHigh] = func(priorityHigh);
		}
		// Free the DLL module.
		FreeLibrary (hinstLib);
	}
	// If unable to call the DLL function, use an alternative.
	if (!RunTimeLinkSuccess) {
		printf("Unable to load callback queue information\n");
	}
	return RunTimeLinkSuccess;
}

/* load_callback_queue_func
 ************************************************************************/
static void check_callback_init() noexcept
{
	// Check if initialized
	if (callback_queue_init) return;
	try {
		// If not get the mutex
		std::lock_guard lock(callback_queue_mux);
		// Check again!
		if (callback_queue_init) return;
		// No initialize
		load_callback_queue_func();
		callback_queue_init = true;
	}
	catch (...) {}
}
#else
/* update_callback
 ************************************************************************/
static void update_callback(bool reset = false) noexcept
{
	try {
		std::lock_guard lock(callback_queue_mux);
		static epicsUInt64 last_access{ 0 };
		epicsUInt64 current = epicsMonotonicGet();
		if (current > last_access + 10000000UL) // 10ms
		{
			last_access = current;
			callbackQueueStatus(reset, &callback_status);
		}
	}
	catch (...) {
		;
	}
}
#endif

/* EpicsInterface::get_callback_queue_size
 ************************************************************************/
int EpicsInterface::get_callback_queue_size (int pri) noexcept
{
	if (!plc::System::get().is_ioc_running()) return -1;
#if EPICS_VERSION < 7
#ifdef _MSC_VER
	check_callback_init();
	return ((pri >= 0) && (pri < NUM_CALLBACK_PRIORITIES) && callback_queue[pri]) ?
		epicsRingPointerGetSize(callback_queue[pri]) : 0;
#else
	return 1;
#endif
#else
	if ((pri < 0) || (pri >= NUM_CALLBACK_PRIORITIES)) return 0;
	update_callback();
	return callback_status.size;
#endif
}

/* EpicsInterface::get_callback_queue_used
 ************************************************************************/
int EpicsInterface::get_callback_queue_used (int pri) noexcept
{
	if (!plc::System::get().is_ioc_running()) return -1;
#if EPICS_VERSION < 7
#ifdef _MSC_VER
	check_callback_init();
	if ((pri < 0) || (pri >= NUM_CALLBACK_PRIORITIES)) return 0;
	const int used = epicsRingPointerGetUsed(callback_queue[pri]);
	if (used > callback_queue_max[pri])  callback_queue_max[pri] = used;
	return used;
#else
	return 0;
#endif
#else
	if ((pri < 0) || (pri >= NUM_CALLBACK_PRIORITIES)) return 0;
	update_callback();
	return callback_status.numUsed[pri];
#endif
}

/* EpicsInterface::get_callback_queue_free
 ************************************************************************/
int EpicsInterface::get_callback_queue_free (int pri) noexcept
{
	if (!plc::System::get().is_ioc_running()) return -1; 
#if EPICS_VERSION < 7
#ifdef _MSC_VER	
	check_callback_init();
	return ((pri >= 0) && (pri < NUM_CALLBACK_PRIORITIES) && callback_queue[pri]) ?
		epicsRingPointerGetFree(callback_queue[pri]) : 0;
#else
	return 0;
#endif
#else
	if ((pri < 0) || (pri >= NUM_CALLBACK_PRIORITIES)) return 0;
	update_callback();
	return callback_status.size - callback_status.numUsed[pri];
#endif
}

/* EpicsInterface::get_callback_queue_highwatermark
 ************************************************************************/
int EpicsInterface::get_callback_queue_highwatermark(int pri) noexcept
{
	if (!plc::System::get().is_ioc_running()) return -1;
#if EPICS_VERSION < 7
#ifdef _MSC_VER
	check_callback_init();
	return ((pri >= 0) && (pri < NUM_CALLBACK_PRIORITIES) && callback_queue[pri]) ?
		callback_queue_max[pri] : 0;
#else
	return 0;
#endif
#else
	if ((pri < 0) || (pri >= NUM_CALLBACK_PRIORITIES)) return 0;
	update_callback();
	return callback_status.maxUsed[pri];
#endif
}

/* EpicsInterface::get_callback_queue_overflow
 ************************************************************************/
int EpicsInterface::get_callback_queue_overflow(int pri) noexcept
{
	if (!plc::System::get().is_ioc_running()) return -1;
#if EPICS_VERSION < 7
#ifdef _MSC_VER
	check_callback_init();
	return ((pri >= 0) && (pri < NUM_CALLBACK_PRIORITIES) && callback_queue[pri]) ?
		callback_queue_max[pri] : 0;
#else
	return 0;
#endif
#else
	if ((pri < 0) || (pri >= NUM_CALLBACK_PRIORITIES)) return 0;
	update_callback();
	return callback_status.numOverflow[pri];
#endif
}

/* EpicsInterface::set_callback_queue_highwatermark_reset
 ************************************************************************/
int EpicsInterface::set_callback_queue_highwatermark_reset() noexcept
{
	if (!plc::System::get().is_ioc_running()) return -1;
#if EPICS_VERSION < 7
#ifdef _MSC_VER
	for (auto& i : callback_queue_max) i = 0;
#endif
#else
	update_callback(true);
#endif
	return 0;
}


extern "C" {
	int get_callback_queue_size(int pri) {
		return EpicsInterface::get_callback_queue_size(pri);
	}
	int get_callback_queue_used(int pri) {
		return EpicsInterface::get_callback_queue_used(pri);
	}
	int get_callback_queue_free(int pri) {
		return EpicsInterface::get_callback_queue_free(pri);
	}
	int get_callback_queue_highwatermark(int pri) {
		return EpicsInterface::get_callback_queue_highwatermark(pri);
	}
	int get_callback_queue_overflow(int pri) {
		return EpicsInterface::get_callback_queue_overflow(pri);
	}
	int set_callback_queue_highwatermark_reset(void) {
		return EpicsInterface::set_callback_queue_highwatermark_reset();
	}
}
/// @endcond

/************************************************************************
 * Create and export device support entry tables (DSETs).
 * These tell EPICS about the functions used for TCat device support.
 ************************************************************************/

// ai record
static devTcDefIn<epics_record_enum::aival> aival_record_tc_dset;
static devTcDefIn<epics_record_enum::airval> airval_record_tc_dset;

// aai record
static devTcDefIn<epics_record_enum::aaival> aaival_record_tc_dset;

// bi record
static devTcDefIn<epics_record_enum::bival> bival_record_tc_dset;
static devTcDefIn<epics_record_enum::birval> birval_record_tc_dset;

// longin record
static devTcDefIn<epics_record_enum::longinval> longinval_record_tc_dset;

// int64int record
#if EPICS_VERSION >= 7
static devTcDefIn<epics_record_enum::int64inval> int64inval_record_tc_dset;
#endif

// mbbi record
static devTcDefIn<epics_record_enum::mbbival> mbbival_record_tc_dset;
static devTcDefIn<epics_record_enum::mbbirval> mbbirval_record_tc_dset;

// mbbiDirect record
static devTcDefIn<epics_record_enum::mbbiDirectval> mbbiDirectval_record_tc_dset;
static devTcDefIn<epics_record_enum::mbbiDirectrval> mbbiDirectrval_record_tc_dset;

// stringin record
static devTcDefIn<epics_record_enum::stringinval> stringinval_record_tc_dset;
static devTcDefIn<epics_record_enum::lsival> lsival_record_tc_dset;

// waveform record
static devTcDefWaveformIn<epics_record_enum::waveformval> waveformval_record_tc_dset;


// ao record
static devTcDefOut<epics_record_enum::aoval> aoval_record_tc_dset;
static devTcDefOut<epics_record_enum::aorval> aorval_record_tc_dset;

// aao record
static devTcDefOut<epics_record_enum::aaoval> aaoval_record_tc_dset;

// bo record
static devTcDefOut<epics_record_enum::boval> boval_record_tc_dset;
static devTcDefOut<epics_record_enum::borval> borval_record_tc_dset;

// longout record
static devTcDefOut<epics_record_enum::longoutval> longoutval_record_tc_dset;

// int64out record
#if EPICS_VERSION >= 7
static devTcDefOut<epics_record_enum::int64outval> int64outval_record_tc_dset;
#endif
// mbbo record
static devTcDefOut<epics_record_enum::mbboval> mbboval_record_tc_dset;
static devTcDefOut<epics_record_enum::mbborval> mbborval_record_tc_dset;

// mbboDirect record
static devTcDefOut<epics_record_enum::mbboDirectval> mbboDirectval_record_tc_dset;
static devTcDefOut<epics_record_enum::mbboDirectrval> mbboDirectrval_record_tc_dset;

// stringout record
static devTcDefOut<epics_record_enum::stringoutval> stringoutval_record_tc_dset;
static devTcDefOut<epics_record_enum::lsoval> lsoval_record_tc_dset;

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

// int64in record
#if EPICS_VERSION >= 7
epicsExportAddress(dset, int64inval_record_tc_dset);  ///< Record processing entry for int64in
#endif

// mbbi record
epicsExportAddress(dset, mbbival_record_tc_dset); ///< Record processing entry for mbbi
epicsExportAddress(dset, mbbirval_record_tc_dset); ///< Record processing entry for raw mbbi

// mbbiDirect record
epicsExportAddress(dset, mbbiDirectval_record_tc_dset); ///< Record processing entry for mbbiDirect
epicsExportAddress(dset, mbbiDirectrval_record_tc_dset); ///< Record processing entry for raw mbbiDirect

// stringin record
epicsExportAddress(dset, stringinval_record_tc_dset); ///< Record processing entry for stringin
epicsExportAddress(dset, lsival_record_tc_dset); ///< Record processing entry for lsi (long string)

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

// int64out record
#if EPICS_VERSION >= 7
epicsExportAddress(dset, int64outval_record_tc_dset);  ///< Record processing entry for int64out
#endif

// mbbo record
epicsExportAddress(dset, mbboval_record_tc_dset); ///< Record processing entry for mbbo
epicsExportAddress(dset, mbborval_record_tc_dset); ///< Record processing entry for raw mbbo

// mbboDirect record
epicsExportAddress(dset, mbboDirectval_record_tc_dset); ///< Record processing entry for mbboDirect
epicsExportAddress(dset, mbboDirectrval_record_tc_dset); ///< Record processing entry for raw mbboDirect

// stringout record
epicsExportAddress(dset, stringoutval_record_tc_dset); ///< Record processing entry for stringout
epicsExportAddress(dset, lsoval_record_tc_dset); ///< Record processing entry for lso (long string)

}
