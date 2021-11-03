#pragma once
#include "stdafx.h"
#define _CRT_SECURE_NO_WARNINGS
#pragma warning (disable : 26812)
#pragma warning (disable : 26495)
#pragma warning (disable: 4996)
#include "epicsVersion.h"
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
#pragma warning (default: 4996)
#pragma warning (default : 26495)
#pragma warning (default : 26812)
#undef _CRT_SECURE_NO_WARNINGS
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
	
/// Regex for indentifying TwinCAT records
const std::regex tc_regex (
	"((tc)://((\\b([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.?)+:(8[0-9][0-9]))/)(\\d{1,9})/(\\d{1,9}):(\\d{1,9})");

/// Regex for indentifying info records
const std::regex info_regex(
	"((tc)://((\\b([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.?)+:(8[0-9][0-9]))/)(info)/([A-Za-z0-9_]+)");


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
	/// Default constructor (adds linkTcRecord entry)
	register_devsup();
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
	friend void complete_io_scan (EpicsInterface*, IOSCANPVT, int);
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

	/** Get the size of the callback ring buffer 
		For this function to return a valid value the EPICS
		distribution needs to be patched. Add the following lines:

			epicsShareFunc epicsRingPointerId tcat_callbackQueue (int Priority)
			{
				return (Priority >= 0) && (Priority < NUM_CALLBACK_PRIORITIES) ?
					callbackQueue[Priority].queue : NULL;
			}

		after the declaration of

			static cbQueueSet callbackQueue[NUM_CALLBACK_PRIORITIES];

		in src\ioc\db\callback.c

		@param pri Priority of ring buffer
		@return size of the callback ring buffer */
	static int get_callback_queue_size (int pri);
	/** Get the used entries in the callback ring buffer
		For this function to return a valid value the EPICS
		distribution needs to be patched. Add the following lines:

		    epicsShareFunc epicsRingPointerId tcat_callbackQueue (int Priority)
		    {
		        return (Priority >= 0) && (Priority < NUM_CALLBACK_PRIORITIES) ?
		            callbackQueue[Priority].queue : NULL;
		    }

		after the declaration of

		    static cbQueueSet callbackQueue[NUM_CALLBACK_PRIORITIES];

		in src\ioc\db\callback.c

		@param pri Priority of ring buffer
		@return used entries in the callback ring buffer */
	static int get_callback_queue_used (int pri);
	/** Get the free entries in the callback ring buffer
		For this function to return a valid value the EPICS
		distribution needs to be patched. Add the following lines:

			epicsShareFunc epicsRingPointerId tcat_callbackQueue (int Priority)
			{
				return (Priority >= 0) && (Priority < NUM_CALLBACK_PRIORITIES) ?
					callbackQueue[Priority].queue : NULL;
			}

		after the declaration of

			static cbQueueSet callbackQueue[NUM_CALLBACK_PRIORITIES];

		in src\ioc\db\callback.c

		@param pri Priority of ring buffer
		@return free entries in the callback ring buffer */
	static int get_callback_queue_free(int pri);

protected:
	/// Reset ioscan use flag
	void ioscan_reset(int bitnum);
	/** Bool indicating passive scan
		true : EPICS record SCAN field is set to PASSIVE */
	bool				isPassive;
	/// Bool indicating whether callback is needed to call dbProcess
	/// true : SCAN = I/O Intr or the record is an out record
	bool				isCallback;
	/// Pointer to the EPICS record
	dbCommon*			pEpicsRecord;
	/// IOSCAN mutex
	std::mutex			ioscanmux;
	/// Pointer to IO scan list
	IOSCANPVT			ioscanpvt;
	/// Scan in progress (bit encoded value from priorities)
	std::atomic<unsigned int>	ioscan_inuse;
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
#if EPICS_VERSION >= 7
	/// 64-bit integer input
	int64inval,
	/// 64-bit integer output
	int64outval,
#endif
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
	/// long string input
	lsival,
	/// long string output
	lsoval,
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
public:
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
public:
	/// Record type: aiRecord, etc.
	typedef typename devTcDefIo<RecType>::rec_type rec_type;
	/// Pointer to record type
	typedef typename devTcDefIo<RecType>::rec_type_ptr rec_type_ptr;

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
public:
	/// Record type: aiRecord, etc.
	typedef typename devTcDefIo<RecType>::rec_type rec_type;
	/// Pointer to record type
	typedef typename devTcDefIo<RecType>::rec_type_ptr rec_type_ptr;

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
public:
	/// Record type: aiRecord, etc.
	typedef typename devTcDefIo <RecType>::rec_type rec_type;
	/// Pointer to record type
	typedef typename devTcDefIo <RecType>::rec_type_ptr rec_type_ptr;

	/// Constructor
	devTcDefWaveformIn();
	/// init callback for read records
	static long init_read_waveform_record (rec_type_ptr prec);
	/// read callback
	static long read_waveform (rec_type_ptr precord);
};

void complete_io_scan(EpicsInterface* epics, IOSCANPVT ioscan, int prio);

}
#include "devTcTemplate.h"