#include "tcComms.h"
#include "infoPlc.h"
#include "ParseTpy.h"
#include "windows.h"
#include "TpyToEpics.h"
#include "TcAdsDef.h"
#include "TcAdsAPI.h"
#include <memory>
#include <filesystem>
#include <chrono>

/** @file tcComms.cpp
	Defines methods for TwinCAT communication.
 ************************************************************************/

using namespace std;
using namespace std::filesystem;
//using namespace std::experimental::filesystem::v1;
using namespace plc;

static bool debug = false;
static bool tcdebug = false;



namespace TcComms {

/** Print an error message for an ADS error return code
	@brief errorPrintf
 ************************************************************************/
void errorPrintf(int nErr) noexcept
{
	static int maxerror = 10;
	static int lasterror = 0;

	if (nErr == lasterror)
	{
		if (!maxerror) return;
		--maxerror;
	}
	else
	{
		maxerror = 10;
		lasterror = nErr;
	}
	/////////////////////////////////////////////////
	// This function prints the proper error message for common ADS return codes.
	// Documentation at: http://infosys.beckhoff.com/english.php?content=../content/1033/tcadscommon/html/ads_returncodes.htm&id=
	/////////////////////////////////////////////////
	if (nErr == 4	) 
		printf("no ADS mailbox was available to process this message\n");
	else if (nErr == 6	) 
		printf("target port not found, Cause: System not switched to RUN\n");
	else if (nErr == 7	) 
		printf("target machine not found, Cause: Missing ADS routes\n");
	else if (nErr == 18	) 
		printf("port disabled\n");
	else if (nErr == 19	) 
		printf("port already connected\n");
	else if (nErr == 24	) 
		printf("Invalid ADS port\n");
	else if (nErr == 1285) 
		printf("ROUTERERR_NOTINITIALIZED\n");
	else if (nErr == 1290) 
		printf("ROUTERERR_NOTACTIVATED\n");
	else if (nErr == 1796) 
		printf("reading/writing not permitted\n");
	else if (nErr == 1808) 
		printf("symbol not found\n");
	else if (nErr == 1813) 
		printf("notification client not registered\n");
	else if (nErr == 1861) 
		printf("timeout elapsed; check ADS routes of sender and receiver and your firewall settings\n");
	else if (nErr == 1910) 
		printf("server is in invalid state\n");
	else if (nErr == 1864) 
		printf("ads-port not opened\n");
	else 
		printf("Error code %i: ~~~MYSTERY ERROR!!!~~~ Go Google \"ADS return codes\"!\n",nErr);
}

/************************************************************************
  TCatInterface
 ************************************************************************/

/* TCatInterface constructor
 ************************************************************************/
TCatInterface::TCatInterface (BaseRecord& dval, const stringcase& name, 
							  unsigned long group, unsigned long offset, 
							  unsigned long nBytes, const stringcase& type, 
							  bool isStruct, bool isEnum)
	: Interface (dval), tCatName(name), tCatType(type), 
	tCatSymbol({ 0,0,0 }), requestNum(0), requestOffs(0)
{
	tCatSymbol.indexGroup = group;
	tCatSymbol.indexOffset = offset;
	tCatSymbol.length = nBytes;
	if (isEnum)	tCatType = "ENUM";
	if (isStruct) record.set_process(false);
}

/* TCatInterface::push
 ************************************************************************/
bool TCatInterface::push() noexcept
{
	return true;
}

/* TCatInterface::pull
 ************************************************************************/
bool TCatInterface::pull() noexcept
{
	return true;
}

/* TCatInterface::get_parent
 ************************************************************************/
TcPLC* TCatInterface::get_parent() noexcept
{ 
	return dynamic_cast<TcPLC*>(record.get_parent()); 
}

/* TCatInterface::get_parent
 ************************************************************************/
const TcPLC* TCatInterface::get_parent() const noexcept
{
	return dynamic_cast<const TcPLC*>(record.get_parent());
}

/* TCatInterface::printTCatVal
 ************************************************************************/
void TCatInterface::printVal (FILE* fp) noexcept
{
	/////////////////////////////////////////////////
	/// This is a function for printing the variable name and value of a record.
	/// Depending on the variable type, the readout from the ADS server is cast
	/// into the proper data type and printed to the output file fp.
	/////////////////////////////////////////////////
	fprintf(fp,"%65s: %15s         ",tCatName.c_str(), tCatType.c_str());

	double				doublePLCVar = 0.0;
	float				floatPLCVar = 0.0;
	signed long long	sllPLCVar = 0;
	signed long int		sliPLCVar = 0;
	signed short int	ssiPLCVar = 0;
	signed char			charPLCVar = 0;
	char				chararrPLCVar[256]{};
	
	TcPLC*				parent = get_parent();
	if (!parent) return;	
	TcPLC::buffer_ptr buf = parent->get_responseBuffer (requestNum);
	if (!buf) return;
	const char* const pTCatVal = buf.get() + requestOffs;
	if (tCatType == "LREAL") {
		doublePLCVar	= *(const double*)pTCatVal;
		fprintf(fp,"%f",doublePLCVar);
	}
	else if (tCatType == "REAL") {
		floatPLCVar		= *(const float*)pTCatVal;
		fprintf(fp,"%f",floatPLCVar);
	}
	else if (tCatType == "LWORD" || tCatType == "LINT" || tCatType == "ULINT" || tCatType == "LTIME") {
		sllPLCVar = *(const signed long long*)pTCatVal;
		fprintf(fp, "%lld", sllPLCVar);
	}
	else if (tCatType == "DWORD" || tCatType == "DINT" || tCatType == "UDINT" ||
		tCatType == "TIME" || tCatType == "TOD" || tCatType == "DATE" ||
		tCatType == "DT" || tCatType == "TIME_OF_DAY" || tCatType == "DATE_AND_TIME")  {
		sliPLCVar		= *(const signed long int*)pTCatVal;
		fprintf(fp,"%ld",sliPLCVar);
	}
	else if (tCatType == "INT" || tCatType == "WORD" || tCatType == "ENUM" || tCatType == "UINT") {
		ssiPLCVar		= *(const signed short int*)pTCatVal;
		fprintf(fp,"%d",ssiPLCVar);
	}
	else if (tCatType == "BOOL" || tCatType == "BYTE" || tCatType == "SINT" || tCatType == "USINT") {
		charPLCVar		= *(const signed char*)pTCatVal;
		fprintf(fp,"%d",charPLCVar);
	}
	else if (tCatType.substr(0,6) == "STRING") {
		strncpy_s(chararrPLCVar, sizeof (chararrPLCVar), (char*)pTCatVal, min (tCatSymbol.length, sizeof(chararrPLCVar)));
		fprintf(fp,"%s",chararrPLCVar);
	}
	else {
		fprintf(fp,"INVALID!!!");
	}
	fprintf(fp,"\n");
}


/************************************************************************
  tcProcWrite
 ************************************************************************/

/* tcProcWrite::~tcProcWrite
 ************************************************************************/
tcProcWrite::~tcProcWrite()
{
	if (ptr) {
		req.clear(); // write all request in the queue
		tcwrite(); // write remainder
		delete [] ptr;
	}
}

/* tcProcWrite::operator=
 ************************************************************************/
tcProcWrite&  tcProcWrite::operator= (tcProcWrite&& tp) noexcept
{
	addr = tp.addr;
	port = tp.port;
	if (ptr) delete [] ptr;
	ptr = tp.ptr; tp.ptr = nullptr;
	data = tp.data; tp.data = nullptr;
	size = tp.size; tp.size = 0;
	alloc = tp.alloc; tp.alloc = 0;
	maxrec = tp.maxrec;
	count = tp.count; tp.count = 0;
	req.clear(); // don't copy the requests!
	return *this;
}

/* tcProcWrite::operator()
 ************************************************************************/
void tcProcWrite::operator () (BaseRecord* prec)
{
	if (!prec || !prec->PlcIsDirty()) return;
	const TCatInterface* const tcat = dynamic_cast<TCatInterface*> (prec->get_plcInterface());
	if (!tcat) return;
	const long len = tcat->get_size();
	if (len <= 0) return;
	if (add (tcat->get_indexGroup(), tcat->get_indexOffset(), len)) {
		prec->PlcReadBinary (read_ptr (len), len);
	}
}

/* tcProcWrite::read_ptr
 ************************************************************************/
void* tcProcWrite::read_ptr (int sz) noexcept
{
	if (!check_alloc (sz)) {
		return nullptr;
	}
	char* ret = data + size;
	size += sz;
	return ret;
}

/* tcProcWrite::add
 ************************************************************************/
bool tcProcWrite::add (long igroup, long ioffs, long sz)
{
	if (count == maxrec) {
		// Just queue it up!
		req.push_back (std::move (*this));
	}
	if (!check_alloc (0)) {
		return false;
	}
	char* p = ptr + count * 3 * sizeof (long);
	memcpy (p, &igroup, sizeof (long));
	memcpy (p + sizeof (long), &ioffs, sizeof (long));
	memcpy (p + 2 * sizeof (long), &sz, sizeof (long));
	++count;
	return true;
}

/* tcProcWrite::check_alloc
 ************************************************************************/
bool tcProcWrite::check_alloc (int extra) noexcept
{
	if (extra < 0) {
		return false;
	}

	size_t unit = maxrec * sizeof (long);
	size_t newalloc = (alloc < 8 * unit) ? 8 * unit : alloc;
	while (3 * unit + size + extra > newalloc) newalloc *= 2;
	if (!ptr || (newalloc > alloc)) {
		char* newp = new (nothrow) char [newalloc];
		if (!newp) {
			return false;
		}
		if (ptr) {
			memcpy (newp, ptr, alloc);
			delete [] ptr;
		}
		ptr = newp;
		data = ptr + 3 * unit;
		alloc = newalloc;
	}
	return true;
}

/* tcProcWrite::tcwrite
 ************************************************************************/
void tcProcWrite::tcwrite() noexcept
{
	if (!ptr || (count == 0)) return;
	if ((count < maxrec) && (size > 0)) {
		memmove (ptr + count * 3 * sizeof (long), data, size);
	}
	// ads write
	char* ret = new (std::nothrow) char [4 * count];
	if (!ret) return;
	unsigned long read = 0;
	const int nErr = AdsSyncReadWriteReqEx2(port, &addr, 0xF081, 
		static_cast<unsigned long>(count),
		static_cast<unsigned long>(sizeof(long)*count), ret, 
		static_cast<unsigned long>(3*sizeof(long)*count + size), ptr, &read);
	try {
		if (nErr && (nErr != 18) && (nErr != 6)) errorPrintf(nErr);
	}
	catch (...) {
		;
	}
	// ready for next transfer
	count = 0;
	size = 0;
}


/************************************************************************
  TcPLC
 ************************************************************************/
constexpr time_t WINDOWS_TICK = 10'000'000;
constexpr time_t SEC_TO_UNIX_EPOCH = 11'644'473'600LL;
static constexpr time_t FileTimeToUnixSeconds(time_t windowsTicks) noexcept
{
	return (time_t)(windowsTicks / WINDOWS_TICK - SEC_TO_UNIX_EPOCH);
}

 /* TcPLC::TcPLC constructor
  ************************************************************************/
TcPLC::TcPLC (std::string tpyPath)
	: addr(), pathTpy(tpyPath), timeTpy(0), checkTpy(false), validTpy(true), nRequest(0),
	scanRateMultiple(default_multiple), cyclesLeft(default_multiple), update_workload (0),
	ads_state (ADSSTATE_INVALID), ads_handle (0), ads_restart (false), nReadPort(0), nWritePort(0),
	nNotificationPort(0), read_active(false), plcId(0)
{
	// modification time
	path fpath(pathTpy);
	//timeTpy = file_time_type::clock::to_time_t (last_write_time (fpath));
	timeTpy = last_write_time(fpath).time_since_epoch().count();

	if (debug) {
		const time_t unixt = FileTimeToUnixSeconds(timeTpy);
		char buf[80] = {};
		struct tm loc;
		if (localtime_s(&loc, &unixt) && asctime_s(buf, sizeof(buf), &loc)) {
			buf[79] = '0';
			printf("Tpy time: %s\n", buf);
		}
	}
	// Set PLC ID and initialize list of PLC instances
	{
		std::lock_guard lock(plcVecMutex);
		plcVec.push_back(this);
		plcId = (unsigned int)plcVec.size() - 1;
	};
};

/* TcPLC::set_addr
 ************************************************************************/
bool TcPLC::set_addr(stringcase netIdStr, int port)
{
	ParseTpy::ads_routing_info ads (netIdStr, port);
	if (!ads.get (addr.netId.b[0], addr.netId.b[1], addr.netId.b[2], 
		addr.netId.b[3], addr.netId.b[4], addr.netId.b[5])) {
		return false;
	}
	addr.port = port;
	if(debug) printf("NetID is %s, port is %i \n",netIdStr.c_str(), port);
	set_name (ads.get());
	return true;
}

/* TcPLC::start
 ************************************************************************/
bool TcPLC::start()
{
	// initialize update scanner
	double ticks = 10.0 / fabs((double)update_scanner_period) * 1000.0;
	if (ticks < 1) ticks = 1;
	{
		guard lock(mux);
		update_workload = (int)((double)records.size() / ticks + 1);
		if (!records.empty()) {
			update_last = records.begin()->second;
		}
	}

	// initialize read and write scanner
	nReadPort = openPort();
	nWritePort = openPort();
	if ((nReadPort == 0) || (nWritePort == 0)) {
		printf("Failed to open ADS ports\n");
		return false;
	}
	// Optain local ADS address if netid is zero
	if ((addr.netId.b[0] == 0) && (addr.netId.b[1] == 0) && (addr.netId.b[2] == 0) &&
		(addr.netId.b[3] == 0) && (addr.netId.b[4] == 0) && (addr.netId.b[5] == 0)) {
		const unsigned short port = addr.port;
		const long nErr = AdsGetLocalAddressEx (nReadPort, &addr);
		if (nErr) {
			errorPrintf(nErr);
			return false;
		}
		addr.port = port;
		if(debug) printf("NetID is %u.%u.%u.%u.%u.%u, port is %u \n",
			addr.netId.b[0], addr.netId.b[1], addr.netId.b[2], 
			addr.netId.b[3], addr.netId.b[4], addr.netId.b[5], port);
	}

	// Setup ADS notifications
	setup_ads_notification();
	// start scanners
	return start_read_scanner() && start_write_scanner() && start_update_scanner();
}


/** Compare two TCat records by their group and offset: compbyOffset
 ************************************************************************/
static bool compByOffset(BaseRecordPtr recA, BaseRecordPtr recB)
{
	const TCatInterface* const a = dynamic_cast<TCatInterface*>(recA->get_plcInterface());
	const TCatInterface* const b = dynamic_cast<TCatInterface*>(recB->get_plcInterface());
	if (!a || !b) return true;
	return ((a->get_indexGroup() <= b->get_indexGroup()) && (a->get_indexOffset() < b->get_indexOffset()));
}

/* Checks is tpy file is valid, ie. hasn't changed
 ************************************************************************/
bool TcPLC::is_valid_tpy() noexcept
{
	if (validTpy && checkTpy.load()) {
		try {
			checkTpy = false; // only check when we switch to online
			path fpath(pathTpy);
			if (exists(fpath)) {
				//time_t modtime = file_time_type::clock::to_time_t(last_write_time(fpath));
				const time_t modtime = last_write_time(fpath).time_since_epoch().count();
				validTpy = (modtime == timeTpy);
			}
			else {
				validTpy = false;
			}
			if (!validTpy) {
				printf("ABORT! Updated tpy file for PLC %s\nRESTART tcioc!\n", name.c_str());
			}
		}
		catch (...)
		{
			;
		}
	}
	return validTpy;
}

/* Build TCat read request groups: TcPLC::optimizeRequests
 ************************************************************************/
bool TcPLC::optimizeRequests()
{
	// TODO: THIS FUNCTION NEEDS A NEW NAME
	if (debug) printf("Forming requests...\n");
	if (records.empty()) {
		return true;
	}

	// Copy records into a list for sorting
	std::list<BaseRecordPtr> recordList;
	for (const auto& it : records) {
		const TCatInterface* const a = dynamic_cast<TCatInterface*>(it.second->get_plcInterface());
		// add tc records to optimize list
		if (a) {
			recordList.push_back(it.second);
		}
		// add all others to non tc list
		else {
			nonTcRecords.insert(it);
		}
	}
	if (debug) printf("Number of info records %i\n", (int)std::ssize(nonTcRecords));

	// Sort record list by group and offset
	recordList.sort(compByOffset);
	bool gap = 0;
	int nextOffs = 0;
	if (recordList.size() == 0) {
		return false;
	}
	TCatInterface* rec = dynamic_cast<TCatInterface*>(
							recordList.begin()->get()->get_plcInterface());
	if (!rec) return false;
	DataPar request{};
	nextOffs = rec->get_indexOffset(); 
	request.indexGroup = rec->get_indexGroup();
	request.indexOffset = rec->get_indexOffset();
	request.length = 0;
	int totalGap = 0;
	double relGap = 0.0;
	int nextLength = 0;
	int nextGap = 0;

	// Iterate through the record list
	for (const auto& it : recordList)
	{
		rec = dynamic_cast<TCatInterface*>(it.get()->get_plcInterface());
		if (!rec) continue;
		if (tcdebug) printf("Processing record: %s\n", rec->get_tCatName().c_str());
	
		const long recGroup = rec->get_indexGroup();
		const long recOffset = rec->get_indexOffset();
		const long recSize = rec->get_size();

		nextGap = recOffset - nextOffs;
		totalGap += nextGap;
		nextLength = request.length + recSize + nextGap;
		relGap = ((double) totalGap) / nextLength;

		// Conditions for making new request
		gap = (recOffset - nextOffs > MAX_SINGLE_GAP_SIZE				// big memory gap
			|| (totalGap > MIN_REL_GAP_SIZE && relGap > MAX_REL_GAP))	// accumulative memory gaps
			|| recGroup != request.indexGroup;							// different index group
		
		// Make new request if gap condition met or if request size too big
		if (request.length + recSize > MAX_REQ_SIZE || gap)
		{
			if (debug) printf("Moving to next request... Gap size is %ld\n", recOffset - nextOffs);
			nRequest++;
			adsGroupReadRequestVector.push_back(request);
			request.indexGroup = recGroup;
			request.indexOffset = recOffset;
			request.length = recSize;
			totalGap = 0;
		}
		else
		{
			request.length = nextLength;
		}
		rec->set_requestNum(nRequest);
		nextOffs = request.indexOffset + request.length; // what the next offset should be for continuous
	}
	// Flush out last request
	adsGroupReadRequestVector.push_back(request);
	if (tcdebug) printf("length: %lu, nextOffs: %d, totalGap: %d\n", request.length, nextOffs, totalGap);

	// Make response buffer
	if (debug) printf("Making buffer...\n");
	for (const auto& i : adsGroupReadRequestVector)
	{
		const size_t bufsize = (size_t)i.length + 4;
		buffer_type* buffer = new (nothrow) buffer_type [bufsize];
		if (buffer) memset(buffer, 0, bufsize);
		adsResponseBufferVector.push_back(buffer_ptr(buffer));
	}

	// Set offset into request buffer for each record
	int reqNum = 0;
	size_t recOffs = 0;
	size_t reqOffs = 0;
	for (const auto& it : recordList)
	{
		rec = dynamic_cast<TCatInterface*>(it.get()->get_plcInterface());
		if (!rec) continue;
		reqNum = rec->get_requestNum();
		recOffs = rec->get_indexOffset();
		reqOffs = adsGroupReadRequestVector[reqNum].indexOffset;
		rec->set_requestOffs (recOffs - reqOffs);

		if (tcdebug) printf("Record %s linked to ADS response buffer.\n",rec->get_tCatName().c_str());
	}

	return true;
}

/* TcPLC::get_responseBuffer
************************************************************************/
TcPLC::buffer_ptr TcPLC::get_responseBuffer(size_t idx) noexcept
{
	return (idx >= 0 && idx < adsResponseBufferVector.size()) ?
		adsResponseBufferVector[idx] : buffer_ptr();
}

 /* TcPLC::printAllRecords
 ************************************************************************/
void TcPLC::printAllRecords()
{
	std::vector<BaseRecordPtr> rlist;
	for (const auto& i : records) {
		if (i.second.get() && i.second->get_plcInterface() && 
			i.second->get_plcInterface()->get_symbol_name()) {
			rlist.push_back(i.second);
		}
	}
	std::sort (rlist.begin(), rlist.end(),
		[](const BaseRecordPtr& p1, const BaseRecordPtr& p2) noexcept {
		return _stricmp (p1->get_plcInterface()->get_symbol_name(), 
						 p2->get_plcInterface()->get_symbol_name()) < 0; });
	int num = 0;
	for (const auto& it : rlist) {
		Interface* iface = it.get()->get_plcInterface();
		if (iface) {
			iface->printVal (stdout);
			++num;
		}
	}
	fprintf (stdout, "Printed %i record values\n", num);
}

/* replace_all
************************************************************************/
static void replace_all (std::string& str, const std::string& from, const std::string& to)
{
	std::string::size_type start_pos = 0;
	while ((start_pos = str.find (from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
}

/* escape_regex
************************************************************************/
static std::string escape_regex (const std::string& regex)
{
	std::string str(regex);
	replace_all (str, "\\", "\\\\");
	replace_all (str, "^", "\\^");
	replace_all (str, ".", "\\.");
	replace_all (str, "$", "\\$");
	replace_all (str, "|", "\\|");
	replace_all (str, "(", "\\(");
	replace_all (str, ")", "\\)");
	replace_all (str, "[", "\\[");
	replace_all (str, "]", "\\]");
	replace_all (str, "*", "\\*");
	replace_all (str, "+", "\\+");
	replace_all (str, "?", "\\?");
	replace_all (str, "/", "\\/");
	return str;
}

/* WildcardToRegex
************************************************************************/
static std::string WildcardToRegex (const std::string& pattern)
{
	std::string str ("^" + escape_regex (pattern) + "$");
	replace_all (str, "\\*", ".*");
	replace_all (str, "\\?", ".");
	return str;
}

/* TcPLC::printRecord
 ************************************************************************/
void TcPLC::printRecord (const std::string& var)
{
	std::vector<BaseRecordPtr> rlist;
	for (const auto& i : records) {
		if (i.second.get() && i.second->get_plcInterface() && 
			i.second->get_plcInterface()->get_symbol_name()) {
			rlist.push_back(i.second);
		}
	}
	std::sort(rlist.begin(), rlist.end(),
		[](const BaseRecordPtr& p1, const BaseRecordPtr& p2) noexcept {
		return _stricmp(p1->get_plcInterface()->get_symbol_name(), 
					    p2->get_plcInterface()->get_symbol_name()) < 0;	});
	int num = 0;
	std::string pattern = WildcardToRegex(var);
	std::regex	exp (pattern);
	for (const auto& it : rlist) {
		Interface* iface = it.get()->get_plcInterface();
		if (iface && std::regex_match (iface->get_symbol_name(), exp)) {
			iface->printVal(stdout);
			++num;
		}
	}
	if (num > 5) fprintf(stdout, "Printed %i record values\n", num);
}

/** Callback for ADS state change
 ************************************************************************/
void __stdcall ADScallback (AmsAddr* pAddr, AdsNotificationHeader* pNotification, 
							unsigned long plcId)
{
	TcPLC* tCatPlcUser = nullptr;
	{
		std::lock_guard lock(TcPLC::plcVecMutex);
		if ((plcId >= 0) && (plcId < TcPLC::plcVec.size())) {
			tCatPlcUser = TcPLC::plcVec[plcId];
		}
	}
	if (tCatPlcUser) {
		const ADSSTATE state = (ADSSTATE) *(USHORT*)pNotification->data;
		tCatPlcUser->set_ads_state(state);
	}
	else {
		printf("Unknown PLC ID %lu\n", plcId);
	}
}

/** TcPLC::set_ads_state
 ************************************************************************/
void TcPLC::set_ads_state(ADSSTATE state) noexcept
{
	if (ads_state.exchange (state) != state) {
		printf ("%s PLC %s\n", state == ADSSTATE_RUN ? "Online" : "Offline", name.c_str());
		checkTpy = (state == ADSSTATE_RUN);
	} 
}

/* TcPLC::setup_ads_notification
 ************************************************************************/
void TcPLC::setup_ads_notification() noexcept
{
	AdsNotificationAttrib   adsNotificationAttrib{};

	// Invoke notification
	adsNotificationAttrib.cbLength       = sizeof(short);
	adsNotificationAttrib.nTransMode     = ADSTRANS_SERVERONCHA;
	adsNotificationAttrib.nMaxDelay      = 0; // in 100ns units
	adsNotificationAttrib.nCycleTime	 = 0; // in 100ns units

	nNotificationPort = openPort ();
	const LONG nErr = AdsSyncAddDeviceNotificationReqEx (nNotificationPort, &addr, 
		ADSIGRP_DEVICE_DATA, ADSIOFFS_DEVDATA_ADSSTATE, 
		&adsNotificationAttrib, ADScallback, plcId, &ads_handle);
	if (nErr) {
		printf ("Unable to establish ADS notifications for %s\n", name.c_str());
		set_ads_state (ADSSTATE_INVALID);
		if (nErr != 18) errorPrintf(nErr);
		ads_restart = true;
	}
	else {
		// set_ads_state (ADSSTATE_RUN);
	}
}

/* TcPLC::remove_ads_notification
 ************************************************************************/
void TcPLC::remove_ads_notification() noexcept
{
	if (ads_handle) {
		try {
			const LONG nErr = AdsSyncDelDeviceNotificationReqEx(nNotificationPort, &addr, ads_handle);
			if (nErr && (nErr != 1813)) errorPrintf(nErr);
		}
		catch (...) {}
	}
	if (nNotificationPort) closePort (nNotificationPort);
}

/* TcPLC::read_scanner
 ************************************************************************/
void TcPLC::read_scanner()
{	
	std::lock_guard	lockit (sync);
	bool read_success = false;
	if ((get_ads_state() == ADSSTATE_RUN) && is_valid_tpy()) {
		for (int request = 0; request <= nRequest; ++request) {
			 //The below works if using AdsOpenPortEx()
			 //Note: this no longer includes error flag so +4 may not be necessary
			unsigned long retsize = 0;
			int nErr = 0;
			nErr = AdsSyncReadReqEx2 (nReadPort, &addr,
				adsGroupReadRequestVector[request].indexGroup,
				adsGroupReadRequestVector[request].indexOffset,
				adsGroupReadRequestVector[request].length+4, // we request additional "error"-flag(long) for each ADS-sub commands
				adsResponseBufferVector[request].get(), 
				&retsize);
			if (!nErr) {
				read_success = true;
			}
			else {
				if (nErr == 18) {
					if (!ads_restart.load()) {
						printf ("Lost PLC %s\n", name.c_str());
					}
					ads_restart = true;
				}
				else if (nErr != 6) {
					errorPrintf(nErr);
				}
			}
		}
	}
	else {
		read_success = false;
	}

	// Update the data time stamp
	if (read_success) update_timestamp();
	read_active = read_success;

	// Check if it's time to do an EPICS read for the slow (read only) records
	bool readAll = false;
	if (cyclesLeft <= 0) readAll = true;
	// Reset countdown until EPICS read
	if (readAll) cyclesLeft = scanRateMultiple;

	// Update all tc records
	for (const auto& recordsEntry : records) {
		BaseRecord* pRecord = recordsEntry.second.get();
		if (!pRecord) continue;
		TCatInterface* tcat = dynamic_cast<TCatInterface*>(pRecord->get_plcInterface());
		if (!tcat) continue;
		const bool isReadOnly = (pRecord->get_access_rights() == access_rights_enum::read_only);
		buffer_type* buffer = adsResponseBufferVector[tcat->get_requestNum()].get();
		if (readAll || !isReadOnly) {
			if (read_success) {
				pRecord->PlcWriteBinary(buffer + tcat->get_requestOffs(), tcat->get_size());
			}
			else {
				pRecord->UserSetValid (false);
			}
		}
	}

	// update non tc records (try using a different cycle to distribute load)
	if (cyclesLeft == 1) {
		for (const auto& it : nonTcRecords) {
			InfoPlc::InfoInterface* iface = dynamic_cast<InfoPlc::InfoInterface*> (it.second->get_plcInterface());
			if (iface) {
				iface->update();
			}
		}
	}

	--cyclesLeft;
}

/* TcPLC::write_scanner()
 ************************************************************************/
void TcPLC::write_scanner()
{
	std::lock_guard	lockit (sync);
	if ((get_ads_state() == ADSSTATE_RUN) && is_valid_tpy()) {
		tcProcWrite proc(addr, nWritePort);
		for_each (proc);
	}

	// update non tc records (try using a different cycle to distribute load)
	for (const auto& it : nonTcRecords) {
		plc::Interface* iface = it.second->get_plcInterface();
		if (iface) {
			iface->push();
		}
	}
}

/* TcPLC::update_scanner()
 ************************************************************************/
void TcPLC::update_scanner()
{
	static time_t last_restart = 0;
	// Set the dirty flag on a few records to make sure they won't go 
	// stale, i.e., EPICS and TwinCAT data values are diverging.
	if (!update_last.get()) return;
	BaseRecordPtr next;
	for (int i = 0; i < update_workload; ++i) {
		if (get_next (next, update_last.get()) && next.get()) {
			next->UserSetDirty();
			update_last = next;
		}
		else {
			/// what!?
			guard lock (mux);
			update_last = records.begin()->second;
		}
	}
	// restart ads callback when needed
	if ((get_ads_state() == ADSSTATE_INVALID) ||
		(is_read_active() && ads_restart.load())) {
		const time_t t = std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
		if (t > last_restart + 10) {
			last_restart = t;
			printf("Reconnect to PLC %s\n", name.c_str());
			remove_ads_notification();
			setup_ads_notification();
			ads_restart = false;
		}
	}
}

/* TcPLC::openPort
 ************************************************************************/
long TcPLC::openPort() noexcept
{
	return AdsPortOpenEx();
}

/* TcPLC::closePort
 ************************************************************************/
void TcPLC::closePort(long nPort) noexcept
{
	AdsPortCloseEx(nPort);
}

/* TcPLC::plcVec
 ************************************************************************/
std::vector<TcPLC*> TcPLC::plcVec;

/* TcPLC::closePort
 ************************************************************************/
std::mutex TcPLC::plcVecMutex;

/************************************************************************ 
  AmsRouterNotification
 ************************************************************************/

/** Callback for AMS router state change
 ************************************************************************/
void __stdcall RouterCall (long nReason)
{
	AmsRouterNotification::set_router_notification((AmsRouterEvent) nReason);
}

/* AmsRouterNotification::gAmsRouterNotification
 ************************************************************************/
AmsRouterNotification AmsRouterNotification::gAmsRouterNotification;

/* AmsRouterNotification::set_router_notification
 ************************************************************************/
void AmsRouterNotification::set_router_notification(AmsRouterEvent routerevent) noexcept {
	gAmsRouterNotification.ams_router_event.store(routerevent); 
	switch (routerevent)
	{
	case AMSEVENT_ROUTERREMOVED:
		// TODO: do something
		break;
	case AMSEVENT_ROUTERSTOP:
		// TODO: do something
		break;
	case AMSEVENT_ROUTERSTART:
		// TODO: do something
		break;
	default:
		break;
	}
}

/* AmsRouterNotification constructor
 ************************************************************************/
AmsRouterNotification::AmsRouterNotification() noexcept
	: ads_version (0), ads_revision (0), ads_build (0),
	ams_router_event (AMSEVENT_ROUTERSTART)
{
	const LONG nErr = AdsGetDllVersion();
	//AdsVersion* pDLLVersion = (AdsVersion *)&nErr;
	ads_version = (int)(nErr & 0x000000FF);// pDLLVersion->version;
	ads_revision = (int)(nErr & 0x0000FF00) >> 8;//pDLLVersion->revision;
	ads_build = (int)(((unsigned int)nErr & 0xFFFF0000U) >> 16); // pDLLVersion->build;
	printf("ADS version: %i, revision: %i, build: %i\n", 
		   ads_version, ads_revision, ads_build);
	return;
	//nErr = AdsPortOpen();
	//nErr = AdsAmsRegisterRouterNotification(&RouterCall);
	//if (nErr) errorPrintf(nErr);
}

/* AmsRouterNotification destructor
 ************************************************************************/
AmsRouterNotification::~AmsRouterNotification()
{
	try {
		const LONG nErr = AdsAmsUnRegisterRouterNotification();
		if (nErr) errorPrintf(nErr);
	}
	catch(...) {}
}

}