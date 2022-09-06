#pragma once
#include "stdafx.h"
#include <TcAdsDef.h>
#include "plcBase.h"

/** @file tcComms.h
	Header which includes classes to interface with the TCat system and 
	manage TCat symbols.
 ************************************************************************/

/** Forward declaration
 ************************************************************************/
struct dbCommon;

/** @namespace TcComms
	TcComms name space, which has all the classes and functions used for 
	communicating with TCat.
	@brief Namespace for TCat communication
 ************************************************************************/
namespace TcComms {

/** @defgroup tccommgroup TwinCAT read/write scanning
 ************************************************************************/
/** @{ */

/// maximum allowed request size (bytes)
constexpr int MAX_REQ_SIZE = 250000;
/// maximum allowed size (bytes) of a memory gap within continuous request
constexpr int MAX_SINGLE_GAP_SIZE = 50;
/// (maximum allowed total gap size) / (current request size)
constexpr double MAX_REL_GAP = 0.25;
/// minimum allowed relative gap size (bytes)
constexpr int MIN_REL_GAP_SIZE = 100;

/// default PLC TwinCAT scan rate (100ms)
constexpr int default_scanrate = 100;
/// minimum PLC TwinCAT scan rate (5ms)
constexpr int minimum_scanrate = 5;	
/// maximum PLC TwinCAT scan rate (10s)
constexpr int maximum_scanrate = 10000;
/// default multiple for PLC EPICS scan rate (10)
constexpr int default_multiple = 10;
/// minimum multiple for PLC EPICS scan rate (1)
constexpr int minimum_multiple = 1;	
/// maximum multiple for PLC EPICS scan rate (200) 
constexpr int maximum_multiple = 200;


/** Forward declaration
 ************************************************************************/
class TcPLC;


/** Struct for storing index group, index offset, and size of a TC symbol
	@brief Memory location struct
 ************************************************************************/
struct DataPar
{
	/// index group in ADS server
	unsigned long		indexGroup;
	/// index offset in ADS server
	unsigned long		indexOffset;
	/// count of bytes to read
	unsigned long		length;
};

/** This is a class for a TCat interface
	@brief TCat interface class
 ************************************************************************/
class TCatInterface	: public plc::Interface
{
public:
	/// Constructor
	explicit TCatInterface (plc::BaseRecord& dval) noexcept
		: Interface(dval), tCatSymbol({ 0,0,0 }), requestNum (0), 
		requestOffs (0) {};
	/// Constructor
	/// @param dval BaseRecord that this interface is part of
	/// @param name Name of TCat symbol
	/// @param group Index group of TCat symbol
	/// @param offset Index offset of TCat symbol
	/// @param length Size in bytes of TCat symbol
	/// @param type Name of TCat data type
	/// @param isStruct True = this symbol is a structure in TCat
	/// @param isEnum True = this symbol is an enum in TCat
	TCatInterface (plc::BaseRecord& dval, const std::stringcase& name, 
		unsigned long group, unsigned long offset, unsigned long length,
		const std::stringcase& type, bool isStruct, bool isEnum);

	/// Get name of TCat symbol
	const std::stringcase get_tCatName() const noexcept {
		return tCatName; };							
	/// Set name of TCat symbol
	void set_tCatName(std::stringcase name) noexcept {
		tCatName = name; };
	/// Get symbol name
	const char* get_symbol_name() const noexcept override {
		return tCatName.c_str(); }
	/// Get TCat data type
	const std::stringcase get_tCatType() noexcept {
		return tCatType; };
	/// Set TCat data type
	void set_tCatType(std::stringcase type) noexcept {
		tCatType = type; };
	/// Get structure containing index group, index offset, size
	DataPar	get_tCatSymbol() noexcept {
		return tCatSymbol; };
	/// Get index group
	unsigned long get_indexGroup() const noexcept {
		return tCatSymbol.indexGroup; };
	/// Set index group
	void set_indexGroup(unsigned long group) noexcept {
		tCatSymbol.indexGroup = group; };
	/// Get index offset
	unsigned long get_indexOffset() const noexcept {
		return tCatSymbol.indexOffset; };
	/// Set index offset
	void set_indexOffset(unsigned long offset) noexcept {
		tCatSymbol.indexOffset = offset; };
	/// Get size in bytes of symbol
	unsigned long get_size() const noexcept {
		return tCatSymbol.length; };
	/// Set size in bytes of symbol
	void set_size(unsigned long nBytes) noexcept {
		tCatSymbol.length = nBytes; };
	/// Get offset into response buffer
	size_t get_requestOffs() const noexcept {
		return requestOffs; };
	/// Set offset into response buffer
	void set_requestOffs(size_t pVal) noexcept {
		requestOffs = pVal; };
	/// Get parent PLC that owns this record
	virtual TcPLC* get_parent() noexcept;
	/// Get parent PLC that owns this record
	virtual const TcPLC* get_parent() const noexcept;
	/// Get the request group number this record is in
	int	get_requestNum() noexcept {
		return requestNum; };
	/// Set the request group number this record is in
	void set_requestNum(int rNum) noexcept {
		requestNum = rNum; };

	/// Prints TCat symbol value and information
	/// @param fp File to print symbol to
	void printVal (FILE* fp) noexcept override;

	/// Does nothing
	bool push() noexcept override;
	/// Does nothing
	bool pull() noexcept override;
protected:
	/// Name of TCat symbol
	std::stringcase		tCatName;
	/// Data type in TCat
	std::stringcase		tCatType;
	/// Struct storing index group, index offset, and length of TC symbol
	DataPar				tCatSymbol;
	/// Which request group in the PLC
	int					requestNum;
	/// Offset into response buffer
	size_t				requestOffs;
};


/** Class for collecting and processing write requests
	This class iterates through the entire record list on the PLC and 
	collects those records whose data value has a dirty flag set on the 
	plc side. These records are then sent as a group to ADS.

	In order to not overload the ADS server, a maximum number of symbols 
	per request is defined, and should not be > 2000.

	@brief TwinCAT process write requests
 ************************************************************************/
class tcProcWrite 
{
public:
	/// Default constructor
	tcProcWrite (const AmsAddr& a, long amsport, size_t mrec = 1000) noexcept
		: addr (a), port (amsport), ptr (nullptr), data (nullptr), 
		maxrec (mrec), size (0), alloc (0), count (0) {
	}
	/// Destructor: will porcess the TCat writes
	~tcProcWrite();
		/// Move constructor
	tcProcWrite (tcProcWrite&& tp) noexcept 
		: addr({ AmsNetId({0,0,0,0,0,0}),0 }), port(0), ptr(nullptr),
		data (nullptr), maxrec (0), size (0), alloc (0), count (0) {
		*this = std::move (tp); }
	/// Move assignment operator
	tcProcWrite& operator= (tcProcWrite&&) noexcept;

	/// Process on record
	void operator () (plc::BaseRecord* prec);
	/// Get a pointer to read the value in
	/// @param sz Requested size
	void* read_ptr (int sz) noexcept;
	/// Add header info
	/// @param igroup iGroup number for tc write
	/// @param ioffs  iOffset number for tc write
	/// @param sz Size of data to be written
	/// @return true if succesful
	bool add (long igroup, long ioffs, long sz);

protected:
	/// AMS address
	AmsAddr		addr;
	/// Port to be used to write to TCat
	long		port;
	/// Pointer to header to be written
	char*		ptr;
	/// Pointer to data to be written
	char*		data;
	/// Maximum number of individual requests
	size_t		maxrec;
	/// Size of data
	size_t		size;
	/// Size of allocated header/data array
	size_t		alloc;
	/// Current number of individual requests
	size_t		count;
	/// Queued TCat requests 
	/// (each would have reached maxrec individual requests)
	std::vector<tcProcWrite> req;

	/// Checks if we have enough memory allocated
	bool check_alloc (int extra = 0) noexcept;
	/// writes the current header/data to TCat
	void tcwrite() noexcept;

private:
	/// Copy constructor (disabled)
	tcProcWrite (const tcProcWrite&) = delete;
	/// Assignment operator (disabled)
	tcProcWrite&  operator= (const tcProcWrite&) = delete;
};


/** Class for a connection to a TwinCAT PLC
	This class is derived from a BasePLC object, and specializes in 
	managing records that contain a plc interface for TCat. This class 
	is initialized using a .tpy file, from which it will obtain the 
	AMS address information for connecting with TCat through ADS.

	Reading and writing from/to ADS will be managed by this class, with 
	read requests being grouped by continuous memory region in TCat to 
	optimize read scanning for speed. Write requests are made using 
	an ADS sum request.

	There is also an option to send a request to ADS to check the status 
	of both the PLC device and also the ADS connection.

	@brief TwinCAT PLC
 ************************************************************************/
class TcPLC	: public plc::BasePLC
{
	/// Notification callback is a friend
	friend void __stdcall ADScallback (AmsAddr*, AdsNotificationHeader*, unsigned long);
public:
	/// Buffer type
	using buffer_type = char;
	/// Smart pointer to buffer
	using buffer_ptr = std::shared_ptr<buffer_type>;

	/// Constructor
	TcPLC(std::string tpyPath);
	/// Destructor
	~TcPLC() override { remove_ads_notification(); };

	/// Is typ still valid? Meaning, it hasn't changed
	bool is_valid_tpy() noexcept;

	/// Get AMS netID of TwinCAT system and port number for this PLC
	AmsAddr	get_addr() const noexcept { return addr; };
	/// Set AMS address
	/// @return true if successful
	bool set_addr(std::stringcase netid, int port);
	/// Get read port number
	long get_nReadPort() const noexcept { return nReadPort; };
	/// Get write port number
	long get_nWritePort() const noexcept { return nWritePort; };
	/// Get slowdown multiple for EPICS read
	int get_read_scanner_multiple() const noexcept {
		return scanRateMultiple; };
	/// Set slowdown multiple for EPICS read
	void set_read_scanner_multiple (int mult) noexcept {
		scanRateMultiple = mult; };
	/// Get ADS state
#pragma warning(disable :26812)
	ADSSTATE get_ads_state() const noexcept { return ads_state.load(); }
#pragma warning(default :26812)
	/// Is read scanner active and successful
	bool is_read_active() const noexcept { return read_active; }

	/// Get the tpy filename
	const std::string& get_tpyfilename() const noexcept {
		return pathTpy; }
	/// Is the tpy file valid?
	bool is_tpyfile_valid() const noexcept {
		return validTpy; }
	/// Get the file modification time of the tpy file
	time_t get_tpyfile_time() const noexcept {
		return timeTpy;
	}

	/// Starts the appropriate scanners
	bool start() override;

	/** Sorts read channels into request groups. Will make a new request 
		group for channels not in continuous memory region in TCat. Will 
		create buffers of appropriate size for each read request, and let 
		each TCat record know where in the read response buffer the data 
		for that symbol is.
		@return true if successful
	*/
	bool optimizeRequests();

	/// Get pointer to the beginning of a read request response buffer
	/// @param idx Index of response buffer
	/// @return pointer to buffer
	buffer_ptr get_responseBuffer(size_t idx) noexcept;

	/// Prints symbol information for entire list of symbols to console
	void printAllRecords() override;
	/// Print a record values to stdout. (override for action)
	/// @param var variable name (accepts wildcards)
	void printRecord(const std::string& var) override;

protected:
	/// Makes read requests to ADS, makes PlcWrite on all data values
	void read_scanner() override;
	/// Collects records to be written to TCat, makes write request
	void write_scanner() override;
	/// Makes sure we don't have stale values.
	void update_scanner() override;
	
	/// Set ADS state
	void set_ads_state(ADSSTATE state) noexcept;
	/// Set up ADS status change notification
	void setup_ads_notification() noexcept;
	/// Remove ADS status change notification
	void remove_ads_notification() noexcept;

	/// Opens a new ADS communication port
	long openPort() noexcept;
	/// Closes an ADS communication port
	/// @param nPort Number of port to close
	void closePort(long nPort) noexcept;

	/// Mutex
	std::mutex	sync;
	/// AMS netID of TwinCAT system and port number for this PLC
	AmsAddr	addr;
	/// The path of the tpy file
	std::string	pathTpy;
	/// Modification time of file
	time_t timeTpy;
	/// need to check modifcation time to make sure tpy file hasn't changed
	std::atomic<bool> checkTpy;
	/// tpy file is valid and hasn't changed
	bool validTpy;

	/// Number of read request groups
	int	nRequest;
	/// Vector of index group, index offset, size for read requests
	std::vector<DataPar> adsGroupReadRequestVector;
	/// Vector of buffers for each read request group
	std::vector<buffer_ptr>	adsResponseBufferVector;
	/// List of all records that don't interface directly with a PLC (info)
	plc::BaseRecordList	nonTcRecords;

	/// Slowdown multiple for EPICS read
	int	scanRateMultiple;
	/** Cycles until EPICS read will be made
		Counts down from scanRateMultiple, resets at 0 */
	int cyclesLeft;
	/// Workload for update scanner
	int update_workload;
	/// last updated record
	plc::BaseRecordPtr update_last;
	/// ADS state
	std::atomic<ADSSTATE> ads_state;
	/// ADS handle
	unsigned long ads_handle;
	/// ADS restart
	std::atomic<bool> ads_restart;

	/// Port number for ADS read connection
	long nReadPort;
	/// Port number for ADS write connection
	long nWritePort;
	/// Port number for ADS notification connection
	long nNotificationPort;
	/// read active and successful
	bool read_active;
private:
	/// Map PLC instance to an integer for use in ADScallback
	static std::vector<TcPLC*> plcVec;
	/// Mutex for PLC instance vector
	static std::mutex plcVecMutex;
	/// PLC ID
	unsigned plcId;
};

/** Class for a AMS router notifications

	@brief AMS Router Notification
 ************************************************************************/
class AmsRouterNotification
{
	/// Notification callback is a friend
	friend void __stdcall RouterCall (long);
public:
	/// get router notification
#pragma warning(disable :26812)
	static AmsRouterEvent get_router_notification() noexcept {
		return gAmsRouterNotification.ams_router_event.load(); };
#pragma warning(default :26812)
	///get global instance
	static const AmsRouterNotification& get_instance() noexcept {
		return gAmsRouterNotification; }

	/// get ADS protocol/library version
	int get_ads_version() const noexcept {
		return ads_version; }
	/// get ADS protocol/library revision
	int get_ads_revision() const noexcept {
		return ads_revision; }
	/// get ADS protocol/library build
	int get_ads_build() const noexcept {
		return ads_build; }
protected:
	/// ADS protocol/library version
	int ads_version;
	/// ADS protocol/library revision
	int ads_revision;
	/// ADS protocol/library build
	int ads_build;
private:
	/// AMS router state
	std::atomic<AmsRouterEvent>	ams_router_event;
	/// Constructor
	AmsRouterNotification() noexcept;
	/// Copy constructor
	AmsRouterNotification (const AmsRouterNotification&) = delete;
	/// Move constructor
	AmsRouterNotification(AmsRouterNotification&&) = delete;
	/// Destructor
	~AmsRouterNotification();
	/// Copy operator
	AmsRouterNotification& operator= (const AmsRouterNotification&) = delete;
	/// Copy move operator
	AmsRouterNotification& operator= (AmsRouterNotification&&) = delete;
	/// set router notification
	static void set_router_notification(AmsRouterEvent routerevent) noexcept;
	/// one global instance
	static AmsRouterNotification gAmsRouterNotification;
};

/** @} */


}

