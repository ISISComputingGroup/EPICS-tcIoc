#pragma once
#include "stdafx.h"
#include "epicsVersion.h"
#include "dbAccess.h"
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
	
/// Regex for indentifying TwinCAT records
inline const std::regex tc_regex (
	"((tc)://((\\b([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.?)+:(8[0-9][0-9]))/)(\\d{1,9})/(\\d{1,9}):(\\d{1,9})");

/// Regex for indentifying info records
inline const std::regex info_regex(
	"((tc)://((\\b([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.?)+:(8[0-9][0-9]))/)(info)/([A-Za-z0-9_]+)");


/** This is a class for managing device support for multiple record
    types, such as TwinCAT/ADS and Info.
    @brief Device support registration.
 ************************************************************************/
class register_devsup
{
public:
	/// Type descriping the link function
	using link_func = bool (&) (dbCommon* pEpicsRecord, plc::BaseRecordPtr& pRecord);
	/// pair of pattern and link function
	using test_pattern = std::pair<std::regex, link_func&>;
	/// list of pattern/link functions
	using test_pattern_list = std::vector<test_pattern>;

	/// Register a pattern/link function
	static void add (const std::regex& rgx, link_func& func) noexcept {
		the_register_devsup.tp_list.push_back (test_pattern (rgx, func)); }

	/// Go through list and call first link function which matches the pattern
	/// Used to link epics records with internal records.
	///	@param inpout Value of INP/OUT field
	/// @param pEpicsRecord Pointer to EPICS record
	/// @param pRecord Pointer to a base record (return)
	/// @return true if one match was found and successfully linked
	/// @brief linkRecord
	static bool linkRecord (const std::stringcase& inpout, dbCommon* pEpicsRecord, 
		plc::BaseRecordPtr& pRecord) noexcept;

protected:
	/// Default constructor (adds linkTcRecord entry)
	register_devsup() noexcept;
	/// Disabled copy constructor
	register_devsup (const register_devsup&) = delete;
	/// Disabled move constructor
	register_devsup(register_devsup&&) = delete;
	/// Disabled assignment operator
	register_devsup& operator= (const register_devsup&) = delete;
	/// Disabled move assignment operator
	register_devsup& operator= (register_devsup&&) = delete;

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
class EpicsInterface : public plc::Interface
{
	friend void complete_io_scan (EpicsInterface*, IOSCANPVT, int) noexcept;
public:
	/// Constructor
	explicit EpicsInterface(plc::BaseRecord& dval) noexcept : Interface(dval) {}

	/// Set isPassive
	void set_isPassive(bool passive) noexcept {
		isPassive = passive; };
	/// Get isCallback
	bool get_isCallback() const noexcept {
		return isCallback; };
	/// Set isCallback
	void set_isCallback(bool isCb) noexcept {
		isCallback = isCb; };
	/// Set pEpicsRecord
	void set_pEpicsRecord(dbCommon* pEpRecord) noexcept {
		pEpicsRecord = pEpRecord;};
	/// Get callbackRequestPending
	bool get_callbackRequestPending() const noexcept;

	/// Get pointer to callback structure
	const epicsCallback& callback() const noexcept {
		return callbackval; }
	/// Get pointer to callback structure
	epicsCallback& callback() noexcept {
		return callbackval; }
	/// Get reference to io scan list pointer
	const IOSCANPVT& ioscan() const noexcept {
		return ioscanpvt; }
	/// Get reference to io scan list pointer
	IOSCANPVT& ioscan() noexcept {
		return ioscanpvt; }
	/// Get pointer to io scan list
	IOSCANPVT get_ioscan() const noexcept {
		return ioscanpvt; }
	/// Set pointer to io scan list
	void set_ioscan (const IOSCANPVT ioscan) noexcept {
		ioscanpvt = ioscan; }

	/// Makes a call to the EPICS dbProcess function
	bool push() noexcept override;
	/// Does nothing
	bool pull() noexcept override { return true; }

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

		in src\ioc\db\callback.c (not needed in EPICS 7)

		@param pri Priority of ring buffer
		@return size of the callback ring buffer */
	static int get_callback_queue_size (int pri) noexcept;
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

		in src\ioc\db\callback.c (not needed in EPICS 7)

		@param pri Priority of ring buffer
		@return used entries in the callback ring buffer */
	static int get_callback_queue_used (int pri) noexcept;
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

		in src\ioc\db\callback.c (not needed in EPICS 7)

		@param pri Priority of ring buffer
		@return free entries in the callback ring buffer */
	static int get_callback_queue_free(int pri) noexcept;
	/** Get the high watermark in the callback ring buffer
		@param pri Priority of ring buffer
		@return high watermark in the callback ring buffer */
	static int get_callback_queue_highwatermark(int pri) noexcept;
	/** Get the number of overflows in the callback ring buffer
		@param pri Priority of ring buffer
		@return number of overflows in the callback ring buffer */
	static int get_callback_queue_overflow(int pri) noexcept;
	/** Reset the overflow count in the callback ring buffer
		@return number of overflows in the callback ring buffer */
	static int set_callback_queue_highwatermark_reset() noexcept;

protected:
	/// Reset ioscan use flag
	void ioscan_reset(int bitnum) noexcept;
	/** Bool indicating passive scan
		true : EPICS record SCAN field is set to PASSIVE */

	bool				isPassive = false;
	/// Bool indicating whether callback is needed to call dbProcess
	/// true : SCAN = I/O Intr or the record is an out record
	bool				isCallback = false;
	/// Pointer to the EPICS record
	dbCommon*			pEpicsRecord = nullptr;
	/// IOSCAN mutex
	std::mutex			ioscanmux;
	/// Pointer to IO scan list
	IOSCANPVT			ioscanpvt = nullptr;
	/// Scan in progress (bit encoded value from priorities)
	std::atomic<unsigned int>	ioscan_inuse = 0;
	/// Callback structure
	epicsCallback		callbackval = {};
};


/** This record type enums are used as index the epics traits class
    @brief Epics record type enum.
 ************************************************************************/
enum class epics_record_enum
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
	using traits_type = struct {
		/// Value
		double val; 
	};
	/// Epics record type pointer
	using traits_type_ptr = epics_record_traits<RecType>::value_type*;
	/// Value type of (raw) value field
	using value_type = epicsFloat64;

	/// Name of the record
	static const char* const name () { return "invalid"; };
    /// return value for read_io functions 0=default, 2=don't convert
    static const int value_conversion = 0;
	/// Indicates if this is an input record
	static const bool input_record = true;
	/// Indicates if this is a raw record
	static const bool raw_record = false;
	/// Returns the (raw) value of a record
	static traits_type_ptr val (traits_type* prec) { return (traits_type_ptr) &prec->val; }
	/// Performs the read access on prec 
	static bool read (traits_type_ptr epicsrec, plc::BaseRecord* baserec) {
		return baserec->UserRead (*val (epicsrec)); }
	/// Performs the write access on prec
	static bool write (plc::BaseRecord* baserec, traits_type_ptr epicsrec) {
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
	using rec_type = epics_record_traits<RecType>::traits_type;
	/// Pointer to record type
	using rec_type_ptr = epics_record_traits<RecType>::traits_type*;
	/// Report support function
	using report_type = long (*) (int level);
	/// Initialization function
	using init_type = long (*) (int after);
	/// Initialization function
	using init_record_type = long (*) (rec_type_ptr prec);
	/// IO/INT support function
	using get_ioint_info_type = long (*) (int cmd, rec_type_ptr prec, IOSCANPVT* piosl);
	/// IO function like read or write
	using io_type = long (*) (rec_type_ptr prec);
	/// Linear conversion support function
	using special_linconv_type = long (*) (rec_type_ptr prec, int after);

	/// Number of support functions
    long			number = 6;
	/// Report support function
	report_type		report_fn = nullptr;
	/// Init support function
	init_type		init_fn = nullptr;
	/// Record init support function
	init_record_type init_record_fn = nullptr;
	/// IO/INT support function
	get_ioint_info_type	get_ioint_info_fn = get_ioint_info;
	/// Read/write support function
	io_type			io_fn = nullptr;
	/// Linear conversion support function
	special_linconv_type special_linconv_fn = nullptr;

protected:
	/// Constructor for IO record
	devTcDefIo(init_record_type ioinit, io_type io) noexcept :
		init_record_fn(ioinit), io_fn (io) {}

	/// IO/INT info callback
	static long get_ioint_info (int cmd, rec_type_ptr prec, IOSCANPVT* ppvt) noexcept;
};

/** Deviced Support Record for TwinCAT/ADS input
    This structure defines the callback functions for the TC device support.
	This is a base class for both read and write records.
    @brief Device support input record.
 ************************************************************************/
template <epics_record_enum RecType>
struct devTcDefIn : public devTcDefIo<RecType>
{
public:
	/// Record type: aiRecord, etc.
	using rec_type = devTcDefIo<RecType>::rec_type;
	/// Pointer to record type
	using rec_type_ptr = devTcDefIo<RecType>::rec_type_ptr;

	/// Constructor
	devTcDefIn() noexcept : devTcDefIo<RecType>::devTcDefIo(init_read_record, read) {}
	/// init callback for read records
	static long init_read_record (rec_type_ptr prec) noexcept;
	/// read callback
	static long read (rec_type_ptr precord) noexcept;
};

/** Deviced Support Record for TwinCAT/ADS output
    This structure defines the callback functions for the TC device support.
	This is a base class for both read and write records.
    @brief device support output record.
 ************************************************************************/
template <epics_record_enum RecType>
struct devTcDefOut : public devTcDefIo<RecType>
{
public:
	/// Record type: aiRecord, etc.
	using rec_type = devTcDefIo<RecType>::rec_type;
	/// Pointer to record type
	using rec_type_ptr = devTcDefIo<RecType>::rec_type_ptr;

	/// Constructor
	devTcDefOut() noexcept : devTcDefIo<RecType>::devTcDefIo(init_write_record, write) {}
	/// init callback for write records
	static long init_write_record (rec_type_ptr prec) noexcept;
	/// write callback
	static long write (rec_type_ptr precord) noexcept;
};

/** Deviced Support Record for TwinCAT/ADS waveform input
    This structure defines the callback functions for the TC device support.
	This is a base class for both read and write records.
    @brief device support waveform record.
 ************************************************************************/
template <epics_record_enum RecType>
struct devTcDefWaveformIn : public devTcDefIo<RecType>
{
public:
	/// Record type: aiRecord, etc.
	using rec_type = devTcDefIo<RecType>::rec_type;
	/// Pointer to record type
	using rec_type_ptr = devTcDefIo<RecType>::rec_type_ptr;

	/// Constructor
	devTcDefWaveformIn() noexcept : devTcDefIo<RecType>::devTcDefIo(init_read_waveform_record, read_waveform) {}
	/// init callback for read records
	static long init_read_waveform_record (rec_type_ptr prec) noexcept;
	/// read callback
	static long read_waveform (rec_type_ptr precord) noexcept;
};

void complete_io_scan(EpicsInterface* epics, IOSCANPVT ioscan, int prio) noexcept;

}
#include "devTcTemplate.h"