#include "plcBase.h"
#include <windows.h>

/** @file plcBase.cpp
	Defines methods for the internal record entry.
 ************************************************************************/

namespace plc {

System System::tCat;

/* Interface::get_parent
 ************************************************************************/
BasePLC* Interface::get_parent() noexcept
{
	return record.get_parent();
}

/* Interface::get_parent
 ************************************************************************/
const BasePLC* Interface::get_parent() const noexcept
{
	return record.get_parent();
}


/** Will read the value and reset the dirty flag (wstring from string).
   @brief Reset and read
 ************************************************************************/
template<>
bool reset_and_read (DataValueTypeDef::atomic_bool& dirty, 
					 DataValueTypeDef::type_wstring& dest, 
					 const DataValueTypeDef::atomic_string* source) noexcept
{
	// must be before read
	try {
		dirty.store(false, DataValueTypeDef::memory_order);
		auto s = source->load(DataValueTypeDef::memory_order);
		// conversion only works with simple acsii strings; not UTF-8
		dest = DataValueTypeDef::type_wstring(s.begin(), s.end());
		return true;
	}
	catch (...) {
		return false;
	}
}

/** Will set the dirty bit, when the newly written value is different 
   from the old one (string to wstring).
   @brief Write and test
 ************************************************************************/
template<>
bool write_and_test (DataValueTypeDef::atomic_bool& dirty, 
					 const DataValueTypeDef::atomic_bool& read_pending, 
					 DataValueTypeDef::atomic_bool& valid, 
					 DataValueTypeDef::atomic_wstring* dest, 
					 const DataValueTypeDef::type_string& source) noexcept
{
	if (read_pending.load (DataValueTypeDef::memory_order)) return false;
	try {
		// conversion only works with simple acsii strings; not UTF-8
		DataValueTypeDef::type_wstring s(source.begin(), source.end());
		auto old = dest->exchange(s, DataValueTypeDef::memory_order);
		valid.store(true, DataValueTypeDef::memory_order);
		if (old != s) {
			// must be after modifying the value
			dirty.store(true, DataValueTypeDef::memory_order);
		}
		return true;
	}
	catch (...) {
		return false;
	}
}


/* DataValue destructor
 ************************************************************************/
DataValue::~DataValue()
{
	Init (data_type_enum::dtInvalid);
}

/* DataValue copy constructor
 ************************************************************************/
DataValue::DataValue (const DataValue& dval) noexcept
: mydata (nullptr), mysize(0), mytype (data_type_enum::dtInvalid), myvalid (false),
	myuserdirty (false), myplcdirty (false)
{
	*this = dval;
}

/* DataValue assignment
 ************************************************************************/
DataValue& DataValue::operator= (const DataValue& dval) noexcept
{
	Init (dval.mytype, dval.mysize);
	if (!mydata) {
		return *this;
	}
	switch (mytype) 
	{
	case data_type_enum::dtInvalid:
		break;
	case data_type_enum::dtBool:
		*(type_bool*) mydata = *(type_bool*)dval.mydata;
		break;
	case data_type_enum::dtInt8:
		*(type_int8*) mydata = *(type_int8*)dval.mydata;
		break;
	case data_type_enum::dtUInt8:
		*(type_uint8*) mydata = *(type_uint8*)dval.mydata;
		break;
	case data_type_enum::dtInt16:
		*(type_int16*) mydata = *(type_int16*)dval.mydata;
		break;
	case data_type_enum::dtUInt16:
		*(type_uint16*) mydata = *(type_uint16*)dval.mydata;
		break;
	case data_type_enum::dtInt32:
		*(type_int32*) mydata = *(type_int32*)dval.mydata;
		break;
	case data_type_enum::dtUInt32:
		*(type_uint32*) mydata = *(type_uint32*)dval.mydata;
		break;
	case data_type_enum::dtInt64:
		*(type_int64*) mydata = *(type_int64*)dval.mydata;
		break;
	case data_type_enum::dtUInt64:
		*(type_uint64*) mydata = *(type_uint64*)dval.mydata;
		break;
	case data_type_enum::dtFloat:
		*(type_float*) mydata = *(type_float*)dval.mydata;
		break;
	case data_type_enum::dtDouble:
		*(type_double*) mydata = *(type_double*)dval.mydata;
		break;
	case data_type_enum::dtString:
		*(type_string*) mydata = *(type_string*)dval.mydata;
		break;
	case data_type_enum::dtWString:
		*(type_wstring*) mydata = *(type_wstring*)dval.mydata;
		break;
	case data_type_enum::dtBinary:
		memcpy (mydata, (const type_binary)mydata, mysize);
		break;
	}
	myuserdirty.store (dval.myuserdirty.load(), DataValueTypeDef::memory_order);
	myplcdirty.store (dval.myplcdirty.load(), DataValueTypeDef::memory_order);
	myvalid.store (dval.myvalid.load(), DataValueTypeDef::memory_order);
	return *this;
}


/* DataValue::Init (not really MT safe)
 ************************************************************************/
void DataValue::Init (data_type_enum rt, size_type len) noexcept
{
	if (rt == mytype) {
		if ((mytype == data_type_enum::dtInvalid) && !mydata) return;
		if ((mytype != data_type_enum::dtInvalid) && mydata) return;
	}
	if (mydata) {
		if (mytype == data_type_enum::dtBinary) {
			delete [] (char*) mydata;
		}
		else {
			delete (char*) mydata;
		}
	}
	mydata = nullptr;
	mytype = rt;
	switch (mytype) 
	{
	case data_type_enum::dtInvalid:
		break;
	case data_type_enum::dtBool:
		mydata = (data_type) new (std::nothrow) atomic_bool;
		*(atomic_bool*)mydata = false;
		mysize = sizeof (bool);
		break;
	case data_type_enum::dtInt8:
		mydata = (data_type) new (std::nothrow) atomic_int8;
		*(atomic_int8*)mydata = 0;
		mysize = 1;
		break;
	case data_type_enum::dtUInt8:
		mydata = (data_type) new (std::nothrow) atomic_uint8;
		*(atomic_uint8*)mydata = 0;
		mysize = 1;
		break;
	case data_type_enum::dtInt16:
		mydata = (data_type) new (std::nothrow) atomic_int16;
		*(atomic_int16*)mydata = 0;
		mysize = 2;
		break;
	case data_type_enum::dtUInt16:
		mydata = (data_type) new (std::nothrow) atomic_uint16;
		*(atomic_uint16*)mydata = 0;
		mysize = 2;
		break;
	case data_type_enum::dtInt32:
		mydata = (data_type) new (std::nothrow) atomic_int32;
		*(atomic_int32*)mydata = 0;
		mysize = 4;
		break;
	case data_type_enum::dtUInt32:
		mydata = (data_type) new (std::nothrow) atomic_uint32;
		*(atomic_uint32*)mydata = 0;
		mysize = 4;
		break;
	case data_type_enum::dtInt64:
		mydata = (data_type) new (std::nothrow) atomic_int64;
		*(atomic_int64*)mydata = 0;
		mysize = 8;
		break;
	case data_type_enum::dtUInt64:
		mydata = (data_type) new (std::nothrow) atomic_uint64;
		*(atomic_uint64*)mydata = 0;
		mysize = 8;
		break;
	case data_type_enum::dtFloat:
		mydata = (data_type) new (std::nothrow) atomic_float;
		*(atomic_float*)mydata = 0;
		mysize = 4;
		break;
	case data_type_enum::dtDouble:
		mydata = (data_type) new (std::nothrow) atomic_double;
		*(atomic_double*)mydata = 0;
		mysize = 8;
		break;
	case data_type_enum::dtString:
		mydata = (data_type) new (std::nothrow) atomic_string;
		mysize = sizeof (atomic_string);
		break;
	case data_type_enum::dtWString:
		mydata = (data_type) new (std::nothrow) atomic_wstring;
		mysize = sizeof (atomic_wstring);
		break;
	case data_type_enum::dtBinary:
		mydata = (data_type) new (std::nothrow) char[len];
		mysize = len;
		break;
	}
	if (!mydata) {
		mytype = data_type_enum::dtInvalid;
		mysize = 0;
	}
	myuserdirty.store (false, DataValueTypeDef::memory_order);
	myplcdirty.store (false, DataValueTypeDef::memory_order);
}

/* DataValue::Read (type_string)
 ************************************************************************/
bool DataValue::Read (atomic_bool& dirty, type_string& data) const noexcept
{
	try {
		switch (mytype) {
		case data_type_enum::dtString:
			return reset_and_read(dirty, data, (const atomic_string*)mydata);
		default:
			return false;
		}
	}
	catch (...) {
		return false;
	}
}

/* DataValue::Read (type_wstring)
 ************************************************************************/
bool DataValue::Read (atomic_bool& dirty, type_wstring& data) const noexcept
{
	try {
		switch (mytype) {
		case data_type_enum::dtString: 
			// conversion only works with simple acsii strings; not UTF-8
			return reset_and_read (dirty, data, (const atomic_string*)mydata);
		case data_type_enum::dtWString:
			return reset_and_read(dirty, data, (const atomic_wstring*)mydata);
		default:
			return false;
		}
	}
	catch (...) {
		return false;
	}
}

/* DataValue::Read (type_string_value*)
 ************************************************************************/
bool DataValue::Read (atomic_bool& dirty, type_string_value* data, size_type max) const noexcept
{
	if (!data || (max <= 0)) {
		return false;
	}
	type_string d;
	if (!Read (dirty, d)) return false;
	const errno_t err = strncpy_s (data, max, d.c_str(), max - 1);
	data[max-1] = 0;
	return err == 0;
}

/* DataValue::Read (type_wstring_value*)
 ************************************************************************/
bool DataValue::Read (atomic_bool& dirty, type_wstring_value* data, size_type max) const noexcept
{
	if (!data || (max <= 0)) {
		return false;
	}
	type_wstring d;
	if (!Read (dirty, d)) return false;
	const errno_t err = wcsncpy_s (data, max, d.c_str(), max - 1);
	data[max-1] = 0;
	return err == 0;
}

/* DataValue::Write (type_string)
 ************************************************************************/
bool DataValue::Write (atomic_bool& dirty, const atomic_bool& pend, 
					   const type_string& data) noexcept
{
	switch (mytype) {
	case data_type_enum::dtString:
		return write_and_test (dirty, pend, myvalid, (atomic_string*)mydata, data);
	case data_type_enum::dtWString:
		// conversion only works with simple acsii strings; not UTF-8
		return write_and_test (dirty, pend, myvalid, (atomic_wstring*)mydata, data);
	default:
		return false;
	}
}

/* DataValue::Write (type_wstring)
 ************************************************************************/
bool DataValue::Write (atomic_bool& dirty, const atomic_bool& pend, 
					   const type_wstring& data) noexcept
{
	switch (mytype) {
	case data_type_enum::dtWString:
		return write_and_test (dirty, pend, myvalid, (atomic_wstring*)mydata, data);
	default:
		return false;
	}
}

/* DataValue::Write (type_string_value)
 ************************************************************************/
bool DataValue::Write (atomic_bool& dirty, const atomic_bool& pend, 
					   const type_string_value* data, size_type max) noexcept
{
	if (!data || (max <= 0)) {
		return false;
	}
	const size_t len = strnlen (data, max);
	type_string d (data, len);
	return Write (dirty, pend, d);
}

/* DataValue::Write (type_wstring_value)
 ************************************************************************/
bool DataValue::Write (atomic_bool& dirty, const atomic_bool& pend, 
					   const type_wstring_value* data, size_type max) noexcept
{
	if (!data || (max <= 0)) {
		return false;
	}
	const size_t len = wcsnlen (data, max);
	type_wstring d (data, len);
	return Write (dirty, pend, d);
}

/* DataValue::ReadBinary
 ************************************************************************/
DataValue::size_type 
DataValue::ReadBinary (atomic_bool& dirty, type_binary p, size_type len) const noexcept
{
	if ((mytype == data_type_enum::dtInvalid) || !mydata || !p) {
		return 0;
	}
	switch (mytype) 
	{
	case data_type_enum::dtBool:
		return ((len == mysize) && Read (dirty, *(type_bool*)p)) ? mysize : 0;
	case data_type_enum::dtInt8:
		return ((len == mysize) && Read (dirty, *(type_int8*)p)) ? mysize : 0;
	case data_type_enum::dtUInt8:
		return ((len == mysize) && Read (dirty, *(type_uint8*)p)) ? mysize : 0;
	case data_type_enum::dtInt16:
		return ((len == mysize) && Read (dirty, *(type_int16*)p)) ? mysize : 0;
	case data_type_enum::dtUInt16:
		return ((len == mysize) && Read (dirty, *(type_uint16*)p)) ? mysize : 0;
	case data_type_enum::dtInt32:
		return ((len == mysize) && Read (dirty, *(type_int32*)p)) ? mysize : 0;
	case data_type_enum::dtUInt32:
		return ((len == mysize) && Read (dirty, *(type_uint32*)p)) ? mysize : 0;
	case data_type_enum::dtInt64:
		return ((len == mysize) && Read (dirty, *(type_int64*)p)) ? mysize : 0;
	case data_type_enum::dtUInt64:
		return ((len == mysize) && Read (dirty, *(type_uint64*)p)) ? mysize : 0;
	case data_type_enum::dtFloat:
		return ((len == mysize) && Read (dirty, *(type_float*)p)) ? mysize : 0;
	case data_type_enum::dtDouble:
		return ((len == mysize) && Read (dirty, *(type_double*)p)) ? mysize : 0;
	case data_type_enum::dtString:
		return Read (dirty, (type_string_value*) p, len) ? len : 0;
	case data_type_enum::dtWString:
		return Read (dirty, (type_wstring_value*) p, len / 2) ? 2 * int (len / 2)  : 0;
	case data_type_enum::dtBinary:
		if (len != mysize) {
			return 0;
		}
		// Really need a mutex here to protect the memcpy.
		// However, this is currently not used.
		dirty.store (false, DataValueTypeDef::memory_order); // must be first
		memcpy (p, (const type_binary)mydata, len);
		return mysize;
	default:
		return 0;
	}
}

/* DataValue::WriteBinary
 ************************************************************************/
DataValue::size_type 
DataValue::WriteBinary (atomic_bool& dirty, const atomic_bool& pend, 
						const type_binary p, size_type len) noexcept
{
	if ((mytype == data_type_enum::dtInvalid) || !mydata || !p) {
		return 0;
	}
	switch (mytype) 
	{
	case data_type_enum::dtBool:
		return ((len == mysize) && Write (dirty, pend, *(const type_bool*)p)) ? mysize : 0;
	case data_type_enum::dtInt8:
		return ((len == mysize) && Write (dirty, pend, *(const type_int8*)p)) ? mysize : 0;
	case data_type_enum::dtUInt8:
		return ((len == mysize) && Write (dirty, pend, *(const type_uint8*)p)) ? mysize : 0;
	case data_type_enum::dtInt16:
		return ((len == mysize) && Write (dirty, pend, *(const type_int16*)p)) ? mysize : 0;
	case data_type_enum::dtUInt16:
		return ((len == mysize) && Write (dirty, pend, *(const type_uint16*)p)) ? mysize : 0;
	case data_type_enum::dtInt32:
		return ((len == mysize) && Write (dirty, pend, *(const type_int32*)p)) ? mysize : 0;
	case data_type_enum::dtUInt32:
		return ((len == mysize) && Write (dirty, pend, *(const type_uint32*)p)) ? mysize : 0;
	case data_type_enum::dtInt64:
		return ((len == mysize) && Write (dirty, pend, *(const type_int64*)p)) ? mysize : 0;
	case data_type_enum::dtUInt64:
		return ((len == mysize) && Write (dirty, pend, *(const type_uint64*)p)) ? mysize : 0;
	case data_type_enum::dtFloat:
		return ((len == mysize) && Write (dirty, pend, *(const type_float*)p)) ? mysize : 0;
	case data_type_enum::dtDouble:
		return ((len == mysize) && Write (dirty, pend, *(const type_double*)p)) ? mysize : 0;
	case data_type_enum::dtString:
		return Write (dirty, pend, (const type_string_value*) p, len) ? len : 0;
	case data_type_enum::dtWString:
		return Write (dirty, pend, (const type_wstring_value*) p, len / 2) ? 2 * int (len / 2)  : 0;
	case data_type_enum::dtBinary:
		if (len != mysize) {
			return 0;
		}
		if (pend.load(DataValueTypeDef::memory_order)) return false;
		// Really need a mutex here to protect the memcpy; also doesn't
		// check if the new value is different form the old, always sets dirty.
		// However, this is currently not used.
		memcpy ((type_binary)mydata, p, len);
		dirty.store (false, DataValueTypeDef::memory_order); // must be last
		return mysize;
	default:
		return 0;
	}
}

/* DataValue::set_valid
 ************************************************************************/
void DataValue::SetValid (atomic_bool& dirty, bool valid) noexcept
{

	const bool old = myvalid.exchange (valid, DataValueTypeDef::memory_order);
	if (old != valid) {
		// must be after modifying the value
		dirty.store (true, DataValueTypeDef::memory_order); 
	}
}

/* DataValue::get_valid
 ************************************************************************/
bool DataValue::GetValid (atomic_bool& dirty) const noexcept
{
	// must be before read
	dirty.store (false, DataValueTypeDef::memory_order); 
	return myvalid.load (DataValueTypeDef::memory_order);
}


/************************************************************************/
/* BaseRecord */
/************************************************************************/

/* BaseRecord::get_timestamp
 ************************************************************************/
BasePLC::time_type BaseRecord::get_timestamp() const noexcept
{
	if (!parent) return 0;
	return parent->get_timestamp();
}

/************************************************************************/
/* BasePLC */
/************************************************************************/

/* BasePLC::BasePLC
 ************************************************************************/
BasePLC::BasePLC() noexcept
	: timestamp (0), read_scanner_period (1000), write_scanner_period (1000),
	update_scanner_period (1000), scanners_active (false)
{
	records.max_load_factor (0.5);
}

/* BasePLC::add
 ************************************************************************/
bool BasePLC::add (BaseRecordPtr precord)
{
	if (!precord.get()) {
		return false;
	}
	guard lock (mux);
	auto ret = records.insert (
		BaseRecordList::value_type (precord->get_name(), precord));
	precord->set_parent (this);
	return ret.second;
}

/* BasePLC::find
 ************************************************************************/
BaseRecordPtr BasePLC::find (const std::stringcase& name)
{
	guard lock (mux);
	auto i = records.find (name);
	if (i == records.end()) {
		return BaseRecordPtr();
	}
	else {
		return i->second;
	}
}

/* BasePLC::erase
 ************************************************************************/
bool BasePLC::erase (const std::stringcase& name)
{
	guard lock(mux);
	const auto num = records.erase (name);
	return num > 0;
}

/* BasePLC::get_next
 ************************************************************************/
bool BasePLC::get_next (BaseRecordPtr& next, const BaseRecord* prev) const
{
	guard lock (mux);
	if (records.empty() || !prev) {
		return false;
	}
	auto i = records.find (prev->get_name().c_str());
	if (i != records.cend()) {
		++i;
	}
	if (i == records.cend()) {
		next = records.cbegin()->second;
	}
	else {
		next = i->second;
	}
	return true;
}

/* BasePLC::get_timestamp_unix
 ************************************************************************/
static constexpr BasePLC::time_type TICKS_PER_SECOND = 10000000ULL;
static constexpr BasePLC::time_type EPOCH_DIFFERENCE = 11644473600ULL;

time_t BasePLC::get_timestamp_unix() const noexcept
{
    time_type temp;
	//convert from 100ns intervals to seconds
    temp = timestamp / TICKS_PER_SECOND; 
	// too early? return 0
	if (temp < EPOCH_DIFFERENCE) {
		return 0;
	}
	//subtract number of seconds between epochs
	else {
		return (time_t) (temp - EPOCH_DIFFERENCE);
	}
}

/* BasePLC::update_timestamp
 ************************************************************************/
void BasePLC::update_timestamp() noexcept
{
	GetSystemTimeAsFileTime ((LPFILETIME )&timestamp);
}

/* BasePLC::count
 ************************************************************************/
int BasePLC::count() const noexcept
{
	try {
		guard lock(mux);
		return (int)records.size();
	}
	catch (...) {
		return 0;
	}
}

/* BasePLC::test
 ************************************************************************/
void BasePLC::user_data_set_valid (bool valid) noexcept
{
	for_each (
		[&valid](BaseRecord* rec) noexcept {
			rec->UserSetValid (valid);
	});
}

/* BasePLC::test
 ************************************************************************/
void BasePLC::plc_data_set_valid (bool valid) noexcept
{
	for_each (
		[&valid](BaseRecord* rec) noexcept {
			rec->PlcSetValid (valid);
	});
}

/** Structure for arguments sent to a scanner thread
    @brief Scanner thread arguments
 ************************************************************************/
using scanner_thread_args = struct
{ 
	/// PLC this scanner operates on
	plc::BasePLC* plc; 
	/// Period in ms of the scanner
	long scanperiod;
	/// Address of the scanner function to be used (read, write, or upddate)
	plc::BasePLC::scanner_func scanner; 
};

/** Scanner thread callback with periodic timer
	@brief Scanner thread callback
 ************************************************************************/
VOID CALLBACK ScannerProc (
   LPVOID lpArg,               // Data value
   DWORD dwTimerLowValue,      // Timer low value
   DWORD dwTimerHighValue )    // Timer high value

{
	scanner_thread_args* scan = ((scanner_thread_args*) lpArg);
	if (scan->plc->is_scanner_active()) {
		(scan->plc->*(scan->scanner))();
	}
}

/** Scanner thread with periodic timer
	This function uses the windows waitable timer which will call a
	completion routine at a regular interval. The completion routine in
	this case is one of either read_scanner, write_scanner, or
	update_scanner.
    @brief Scanner thread
************************************************************************/
DWORD WINAPI scannerThread (scanner_thread_args args) noexcept
{
	HANDLE				hTimer = 0;
	LARGE_INTEGER		due_time{};

	// Default security attributes, auto-reset timer, unnamed
	hTimer = CreateWaitableTimer (NULL, FALSE, NULL); 
	if (hTimer == NULL) {

		printf("CreateWaitableTimer failed with error %lu\n", GetLastError());
		return 1;
	}
	// Create an integer that will be used to signal the timer 
	// 3 seconds from now.
	due_time.QuadPart = -1 * (LONGLONG) TICKS_PER_SECOND;
	if (!SetWaitableTimer (hTimer, &due_time, args.scanperiod, 
						   ScannerProc, &args, FALSE)) {
		printf("SetWaitableTimer failed with error %lu\n", GetLastError());
		CloseHandle (hTimer);
		return 1;
	}
	// Wait forever and put thread in an alertable state
	while (true) {
		SleepEx (INFINITE, true);
	}
	CloseHandle (hTimer);
	return 1;
}

/* BasePLC::start_read_scanner
 ************************************************************************/
bool BasePLC::start_read_scanner() noexcept
{
	scanner_thread_args args = {this, read_scanner_period, &BasePLC::read_scanner};
	try {
		read_thread = std::thread (scannerThread, args);
		read_thread.detach();
	}
	catch (...) {
		return false;
	}
	return true;
}

/* BasePLC::start_write_scanner
 ************************************************************************/
bool BasePLC::start_write_scanner() noexcept
{
	scanner_thread_args args = {this, write_scanner_period, &BasePLC::write_scanner};
	try {
		write_thread = std::thread (scannerThread, args);
		write_thread.detach();
	}
	catch (...) {
		return false;
	}
	return true;
}

/* BasePLC::start_update_scanner
 ************************************************************************/
bool BasePLC::start_update_scanner() noexcept
{
	scanner_thread_args args = {this, update_scanner_period, &BasePLC::update_scanner};
	try {
		update_thread = std::thread (scannerThread, args);
		update_thread.detach();
	}
	catch (...) {
		return false;
	}
	return true;
}

/************************************************************************/
/* System */
/************************************************************************/


/* Constructor
 ************************************************************************/
System::System() noexcept
	: IocRun (false)
{
}

/* Destructor
 ************************************************************************/
System::~System()
{
}

/* System::add
 ************************************************************************/
bool System::add (BasePLCPtr newPLC)
{
	if (!newPLC.get()) {
		return false;
	}
	// TODO: Error: check if this PLC already exists
	guard lock (mux);
	auto ret = PLCs.insert (
		BasePLCList::value_type (newPLC->get_name(), newPLC));
	return ret.second;
}

/* System::printVals
 ************************************************************************/
void System::printVals()
{
	guard lock (mux);

	for (const auto& it : PLCs)	{
		if (it.second.get()) it.second.get()->printAllRecords();
	}
}

/* System::printVal
 ************************************************************************/
void System::printVal(const std::string& var)
{
	guard lock(mux);

	for (const auto& it : PLCs) {
		if (it.second.get()) it.second.get()->printRecord (var);
	}
}

/* System::find
 ************************************************************************/
BasePLCPtr System::find (std::stringcase id)
{
	guard lock (mux);
	auto i = PLCs.find (id);
	if (i == PLCs.end()) {
		return BasePLCPtr();
	}
	else {
		return i->second;
	}
}


void System::start() noexcept
{
	for_each ([] (BasePLC* plc) noexcept {
		plc->set_scanners_active (true);
	});
}

void System::stop() noexcept
{
	for_each ([] (BasePLC* plc) noexcept {
		plc->set_scanners_active (false);
	});
}
}

extern "C" {
	/// Stop TwinCAT
	__declspec(dllexport) void stopTc(void) {
		plc::System::get().stop();
	}
}
