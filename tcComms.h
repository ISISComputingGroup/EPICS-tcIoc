#pragma once
#include "stdafx.h"
#include "TcAdsDef.h"
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

/** @defgroup scansettings Constants related to read/write scanning
 ************************************************************************/
/** @{ */

/// maximum allowed request size (bytes)
const int MAX_REQ_SIZE = 250000;
/// maximum allowed size (bytes) of a memory gap within continuous request
const int MAX_SINGLE_GAP_SIZE = 50;
/// (maximum allowed total gap size) / (current request size)
const double MAX_REL_GAP = 0.25;
/// minimum allowed relative gap size (bytes)
const int MIN_REL_GAP_SIZE = 100;

/// default PLC TwinCAT scan rate (100ms)
const int default_scanrate = 100;
/// minimum PLC TwinCAT scan rate (5ms)
const int minimum_scanrate = 5;	
/// maximum PLC TwinCAT scan rate (10s)
const int maximum_scanrate = 10000;
/// default multiple for PLC EPICS scan rate (10)
const int default_multiple = 10;
/// minimum multiple for PLC EPICS scan rate (1)
const int minimum_multiple = 1;	
/// maximum multiple for PLC EPICS scan rate (200) 
const int maximum_multiple = 200;

/** @} */

/** Forward declaration
 ************************************************************************/
class TcPLC;

/** @defgroup tccommssymbol Classes for describing TC symbol
 ************************************************************************/
/** @{ */

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
class TCatInterface	:	public plc::Interface
{
public:
	/// Constructor
	explicit TCatInterface (plc::BaseRecord& dval)
		: Interface(dval) {} ;
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
	/// Deconstructor
	~TCatInterface() {};

	/// Get name of TCat symbol
	const std::stringcase get_tCatName() { 
		return tCatName; };							
	/// Set name of TCat symbol
	void set_tCatName(std::stringcase name) { 
		tCatName = name; };
	/// Get TCat data type
	const std::stringcase get_tCatType() { 
		return tCatType; };
	/// Set TCat data type
	void set_tCatType(std::stringcase type) { 
		tCatType = type; };
	/// Get structure containing index group, index offset, size
	DataPar	get_tCatSymbol() { 
		return tCatSymbol; };
	/// Get index group
	unsigned long get_indexGroup() const { 
		return tCatSymbol.indexGroup; };
	/// Set index group
	void set_indexGroup(unsigned long group) { 
		tCatSymbol.indexGroup = group; };
	/// Get index offset
	unsigned long get_indexOffset() const {
		return tCatSymbol.indexOffset; };
	/// Set index offset
	void set_indexOffset(unsigned long offset) { 
		tCatSymbol.indexOffset = offset; };
	/// Get size in bytes of symbol
	unsigned long get_size() const { 
		return tCatSymbol.length; };
	/// Set size in bytes of symbol
	void set_size(unsigned long nBytes) { 
		tCatSymbol.length = nBytes; };
	/// Get offset into response buffer
	size_t get_requestOffs() const { 
		return requestOffs; };
	/// Set offset into response buffer
	void set_requestOffs(size_t pVal) { 
		requestOffs = pVal; };
	/// Get parent PLC that owns this record
	TcPLC* get_parent();
	/// Get parent PLC that owns this record
	const TcPLC* get_parent() const;
	/// Get the request group number this record is in
	int	get_requestNum() { 
		return requestNum; };
	/// Set the request group number this record is in
	void set_requestNum(int rNum) { 
		requestNum = rNum; };

	/// Prints TCat symbol value and information
	/// @param fp File to print symbol to
	void printTCatVal(FILE* fp);

	/// Does nothing
	virtual bool push() override;
	/// Does nothing
	virtual bool pull() override;
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

/** @} */

/** @defgroup tccommsplc Classes for managing groups of TC symbols
 ************************************************************************/
/** @{ */

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
	tcProcWrite (const AmsAddr& a, long amsport, size_t mrec = 1000) 
		: addr (a), port (amsport), ptr (nullptr), data (nullptr), 
		maxrec (mrec), size (0), alloc (0), count (0) {
	}
	/// Destructor: will porcess the TCat writes
	~tcProcWrite();
	/// Move constructor
	tcProcWrite (tcProcWrite&& tp) : ptr (nullptr) {
		*this = std::move (tp); }

	/// Process on record
	void operator () (plc::BaseRecord* prec);
	/// Get a pointer to read the value in
	/// @param sz Requested size
	void* read_ptr (int sz);
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
	bool check_alloc (int extra = 0);
	/// writes the current header/data to TCat
	void tcwrite();

	/// Move operator
	tcProcWrite&  operator= (tcProcWrite&&);
private:
	/// Copy constructor (disabled)
	tcProcWrite (const tcProcWrite&);
	/// Assignment operator (disabled)
	tcProcWrite&  operator= (const tcProcWrite&);
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
class TcPLC	:	public plc::BasePLC
{
	/// Notification callback is a friend
	friend void __stdcall ADScallback (AmsAddr*, AdsNotificationHeader*, unsigned long);
public:
	/// Buffer type
	typedef char						buffer_type;
	/// Smart pointer to buffer
	typedef std::shared_ptr<buffer_type> buffer_ptr;

	/// Constructor
	TcPLC(std::string tpyPath) 
		: pathTpy(tpyPath), nRTS(0), nRequest(0), 
		scanRateMultiple(default_multiple), cyclesLeft(default_multiple), 
		nReadPort(0), nWritePort(0), nNotificationPort (0), read_active (false),
		ads_state (ADSSTATE_INVALID), ads_handle (0), ads_restart (false) {};
	/// Destructor
	~TcPLC() { remove_ads_notification(); };

	/// Get AMS netID of TwinCAT system and port number for this PLC
	AmsAddr	get_addr() { return addr; };
	/// Set AMS address
	/// @return true if successful
	bool set_addr(std::stringcase netid, int port);
	/// Get read port number
	long get_nReadPort() { return nReadPort; };
	/// Get write port number
	long get_nWritePort() { return nWritePort; };
	/// Get run-time system number
	int get_nRTS() { return nRTS; };		
	/// Set run-time system number
	void set_nRTS(int RTS) { nRTS = RTS; };
	/// Set slowdown multiple for EPICS read
	void set_read_scanner_multiple(int mult) { 
		scanRateMultiple = mult; };
	/// Get ADS state
	ADSSTATE get_ads_state() const { return ads_state.load(); }
	/// Is read scanner active and successful
	bool is_read_active() const { return read_active; }

	/// Starts the appropriate scanners
	virtual bool start();

	/** Sorts read channels into request groups. Will make a new request 
		group for channels not in continuous memory region in TCat. Will 
		create buffers of appropriate size for each read request, and let 
		each TCat record know where in the read response buffer the data 
		for that symbol is.
		@return true if successful
	*/
	bool optimizeRequests();

	/// Get pointer to the beginning of a read request response buffer
	buffer_ptr get_responseBuffer (size_t idx) {
		return (idx >= 0 && idx < adsResponseBufferVector.size()) ?  
			adsResponseBufferVector [idx] : buffer_ptr(); }

	/// Prints symbol information for entire list of symbols to console
	virtual void printAllRecords();
protected:
	/// Makes read requests to ADS, makes PlcWrite on all data values
	virtual void read_scanner();
	/// Collects records to be written to TCat, makes write request
	virtual void write_scanner();
	/// Makes sure we don't have stale values.
	virtual void update_scanner();
	
	// Set ADS state
	void set_ads_state(ADSSTATE state);
	// Set up ADS status change notification
	void setup_ads_notification();
	// Remove ADS status change notification
	void remove_ads_notification();

	/// Opens a new ADS communication port
	long openPort();
	/// Closes an ADS communication port
	/// @param Number of port to close
	void closePort(long nPort);

	std::mutex	sync;
	/// AMS netID of TwinCAT system and port number for this PLC
	AmsAddr	addr;
	/// Run-time system number
	int	nRTS;
	/// The path of the tpy file
	std::string	pathTpy;

	/// Number of read request groups
	int	nRequest;
	/// Vector of index group, index offset, size for read requests
	std::vector<DataPar> adsGroupReadRequestVector;
	/// Vector of buffers for each read request group
	std::vector<buffer_ptr>	adsResponseBufferVector;
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
	static AmsRouterEvent get_router_notification() {
		return gAmsRouterNotification.ams_router_event.load(); };
private:
	/// AMS router state
	std::atomic<AmsRouterEvent>	ams_router_event;
	/// Constructor
	AmsRouterNotification();
	/// Destructor
	~AmsRouterNotification();
	/// set router notification
	static void set_router_notification(AmsRouterEvent routerevent);
	/// one global instance
	static AmsRouterNotification gAmsRouterNotification;
};

/** @} */


}

