#pragma once
#include "stdafx.h"
#include "dbAccess.h"
#include "aitTypes.h"
#include "alarm.h"
#include "asDbLib.h"
#include "cvtTable.h"
#include "callback.h"
#include "dbScan.h"
#include "menuFtype.h"
#include "devSup.h"
#include "dbAccessDefs.h"
#include "tcComms.h"

/** @file devTc.h
	Header which includes classes for TwinCAT/ADS device support. 
 ************************************************************************/

/** @namespace DevTc
	DevTc Name space
	@brief Namespace for TCat device support
 ************************************************************************/
namespace DevTc {

/** Callback function to process EPICS out record
	@brief Callback for output record
 ************************************************************************/
	
/// Callback function for output records
inline void outRecordCallback(callbackPvt *pcallback) {
    dbCommon* prec;
	prec = (dbCommon*)((callbackPvt*)(pcallback))->user; 
    if(prec)
        dbProcess(prec);
}

/** Initialization function that matches an EPICS record with an internal
	record entry
	@param name Name of record (INP/OUT field)
	@param pEpicsRecord Pointer to EPICS record
	@param pRecord Pointer to a base record
	@return true if successful
	@brief Link Record
 ************************************************************************/
bool linkRecord (std::stringcase name, dbCommon* pEpicsRecord, plc::BaseRecordPtr& pRecord);

/** Initialization function that matches an EPICS record with an internal
	TwinCAT record entry
	@param pEpicsRecord Pointer to EPICS record
	@param pRecord Pointer to a base record
	@return true if successful
	@brief Link TwinCat Record
 ************************************************************************/
bool linkTcRecord (dbCommon* pEpicsRecord, plc::BaseRecordPtr& pRecord);

/// Regex for indentifying TwinCAT records
const std::regex tc_regex (
	"((tc)://((\\b([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.?)+:(8[0-9][0-9]))/)(\\d{1,9})/(\\d{1,9}):(\\d{1,9})");

/** This is a class for managing device support for multiple record
    types, such as TwinCAT/ADS and Info.
    @brief Device support registration.
 ************************************************************************/
class register_devsup
{
public:
	/// Type descriping the link function
	typedef auto link_func (dbCommon* pEpicsRecord, plc::BaseRecordPtr& pRecord) -> bool;
	/// pair of pattern and link function
	typedef std::pair<std::regex, link_func&> test_pattern;
	/// list of pattern/link functions
	typedef std::vector<test_pattern> test_pattern_list;

	/// Register a pattern/link function
	static void add (const std::regex& rgx, link_func& func) {
		the_register_devsup.tp_list.push_back (test_pattern (rgx, func)); }

	/// Go through list and call first link function which matches the pattern
	/// Used to link epics records with internal records.
	///	@param inpout Value of INP/OUT field
	/// @param pEpicsRecord Pointer to EPICS record
	/// @param pRecord Pointer to a base record (return)
	/// @return true if one match was found and successfully linked
	/// @brief linkRecord
	static bool linkRecord (const std::stringcase& inpout, dbCommon* pEpicsRecord, 
		plc::BaseRecordPtr& pRecord);

protected:
	/// Default constructor; adds a linkTcRecord entry
	register_devsup() {
		add (tc_regex, linkTcRecord); }
	/// Disabled copy constructor
	register_devsup (const register_devsup&);
	/// Disabled assignment operator
	register_devsup& operator= (const register_devsup&);

	/// list of pattern and links
	test_pattern_list	tp_list;
	/// the one global instance of the register class
	static register_devsup the_register_devsup;
};

/** @defgroup devsup Device support for TwinCAT/ADS
 ************************************************************************/
/** @{ */

/** This is a class for an EPICS Interface
    @brief Epics interface class.
 ************************************************************************/
class EpicsInterface	:	public plc::Interface
{
public:
	/// Constructor
	EpicsInterface (plc::BaseRecord& dval);
	/// Deconstructor
	~EpicsInterface() {};

	/// Set isPassive
	void set_isPassive(bool passive) { 
		isPassive = passive; };
	/// Get isCallback
	bool get_isCallback() const { 
		return isCallback; };
	/// Set isCallback
	void set_isCallback(bool isCb) { 
		isCallback = isCb; };
	/// Set pEpicsRecord
	void set_pEpicsRecord(dbCommon* pEpRecord) {
		pEpicsRecord = pEpRecord;};
	/// Get pEpicsVal
	void* get_pEpicsVal() const { 
		return pEpicsVal; };
	/// Set pEpicsVal
	void set_pEpicsVal(void* pVal) { 
		pEpicsVal = pVal;};
	/// Get size
	unsigned long get_size() const { 
		return size; };
	/// Set size
	void set_size (unsigned long nBytes) { 
		size = nBytes; };
	/// Get callbackRequestPending
	bool get_callbackRequestPending() const;

	/// Get pointer to callback structure
	const CALLBACK& callback() const {
		return callbackval; }
	/// Get pointer to callback structure
	CALLBACK& callback() {
		return callbackval; }
	/// Get reference to io scan list pointer
	const IOSCANPVT& ioscan() const {
		return ioscanpvt; }
	/// Get reference to io scan list pointer
	IOSCANPVT& ioscan() {
		return ioscanpvt; }
	/// Get pointer to io scan list
	IOSCANPVT get_ioscan() const {
		return ioscanpvt; }
	/// Set pointer to io scan list
	void set_ioscan (const IOSCANPVT ioscan) {
		ioscanpvt = ioscan; }

	/// Makes a call to the EPICS dbProcess function
	virtual bool push() override;
	/// Does nothing
	virtual bool pull() override { return true; }

protected:
	/** Bool indicating passive scan
		true : EPICS record SCAN field is set to PASSIVE */
	bool				isPassive;
	/// Bool indicating whether callback is needed to call dbProcess
	/// true : SCAN = I/O Intr or the record is an out record
	bool				isCallback;
	/// Pointer to the EPICS record
	dbCommon*			pEpicsRecord;
	/// Pointer to the RVAL field of the EPICS record
	void*				pEpicsVal;
	/// Size (bytes) of the data
	unsigned long		size;
	/** Bool indicating that a read callback is pending
		Set to true for in/out records when callback request is made.
		true : if this is an in/out record, then dbProcess will do an 
		EPICS read instead of write, then reset this value to false */
//+	bool				callbackRequestPending;
	/// Pointer to IO scan list
	IOSCANPVT			ioscanpvt;
	/// Callback structure
	CALLBACK			callbackval;
};


/** This record type enums are used as index the epics traits class
    @brief Epics record type enum.
 ************************************************************************/
enum epics_record_enum
{
	/// double input array
	aaival = 0,
	/// double output array
	aaoval,
	/// double input
	aival,
	/// double output
	aoval,
	/// binary input
	bival,
	/// binary output
	boval,
	/// event
	eventval,
	/// histogram
	histogramval,
	/// integer input
	longinval,
	/// integer output
	longoutval,
	/// enum input
	mbbival,
	/// enum output
    mbboval,
	/// enum input direct
	mbbiDirectval,
	/// enum output direct
	mbboDirectval,
	/// string input
	stringinval,
	/// string output
	stringoutval,
	/// waveform
	waveformval,
	/// raw double input
	airval,
	/// raw double output
	aorval,
	/// raw binary input
	birval,
	/// raw binary output
	borval,
	/// raw enum input
    mbbiDirectrval,
	/// raw enum output
	mbboDirectrval,
	/// raw enum input direct
	mbbirval,
	/// raw enum output direct
	mbborval,
	/// End of enum (sentinel value)
	epics_record_enumEnd,
	/// invalid
	invalidval = -1
};

/** This traits class for Epics records.
    @brief Epics record traits.
 ************************************************************************/
template <epics_record_enum RecType>
struct epics_record_traits
{
	/// Epics record type
	typedef struct { 
		/// Value
		double val; 
	} traits_type;
	/// Value type of (raw) value field
	typedef epicsFloat64 value_type;
	/// Name of the record
	static const char* const name () { return "invalid"; };
	/** Data type of records val/rval field
	    aitEnumInvalid type signals an array. Take type/len from record */
	static const aitEnum value_ait_type = aitEnumFloat64;
	/** Array length: 1=scalar value, 0=array - see nelm for length,
	    fixed for strings according to the record */
	static const aitInt32 value_count = 0;
    /// return value for read_io functions 0=default, 2=don't convert
    static const int value_conversion = 0;
	/// Indicates if this is an input record
	static const bool input_record = true;
	/// Indicates if this is a raw record
	static const bool raw_record = false;
	/// Returns the (raw) value of a record
	static typename value_type* val (traits_type* prec) { return (value_type*) &prec->val; }
	/// Performs the read access on prec 
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) { 
		return baserec->UserRead (*val (epicsrec)); }
	/// Performs the write access on prec
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) { 
		return baserec->UserWrite (*val (epicsrec)); }
};

/** Deviced Support Record for generic TwinCAT/ADS IO
    This structure defines the callback functions for the TC device support.
	This is a base class for both read and write records.
    @brief Device support record.
 ************************************************************************/
template <epics_record_enum RecType>
struct devTcDefIo 
{
	/// Record type: aiRecord, etc.
	typedef typename epics_record_traits<RecType>::traits_type rec_type;
	/// Pointer to record type
	typedef typename rec_type* rec_type_ptr;

	/// Number of support functions
    long		number;
	/// Report support function
    DEVSUPFUN	report_fn;
	/// Init support function
    DEVSUPFUN	init_fn;
	/// Record init support function
    DEVSUPFUN	init_record_fn;
	/// IO/INT support function
    DEVSUPFUN	get_ioint_info_fn;
	/// Read/write support function
    DEVSUPFUN	io_fn;
	/// Linear conversion support function
    DEVSUPFUN	special_linconv_fn;

protected:
	/// Hide constructor
	devTcDefIo();
	/// IO/INT info callback
	static long get_ioint_info (int cmd, dbCommon* prec, IOSCANPVT* ppvt);
};

/** Deviced Support Record for TwinCAT/ADS input
    This structure defines the callback functions for the TC device support.
	This is a base class for both read and write records.
    @brief Device support input record.
 ************************************************************************/
template <epics_record_enum RecType>
struct devTcDefIn : public devTcDefIo <RecType>
{
	/// Constructor
	devTcDefIn();
	/// init callback for read records
	static long init_read_record (rec_type_ptr prec);
	/// read callback
	static long read (rec_type_ptr precord);
};

/** Deviced Support Record for TwinCAT/ADS output
    This structure defines the callback functions for the TC device support.
	This is a base class for both read and write records.
    @brief device support output record.
 ************************************************************************/
template <epics_record_enum RecType>
struct devTcDefOut : public devTcDefIo <RecType>
{
	/// Constructor
	devTcDefOut();
	/// init callback for write records
	static long init_write_record (rec_type_ptr prec);
	/// write callback
	static long write (rec_type_ptr precord);
};

/** Deviced Support Record for TwinCAT/ADS waveform input
    This structure defines the callback functions for the TC device support.
	This is a base class for both read and write records.
    @brief device support waveform record.
 ************************************************************************/
template <epics_record_enum RecType>
struct devTcDefWaveformIn : public devTcDefIo <RecType>
{
	/// Constructor
	devTcDefWaveformIn();
	/// init callback for read records
	static long init_read_waveform_record (rec_type_ptr prec);
	/// read callback
	static long read_waveform (rec_type_ptr precord);
};

}
#include "devTcTemplate.h"