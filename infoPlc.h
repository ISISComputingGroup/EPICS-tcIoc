#pragma once
#include "stdafx.h"
#include "ParseUtil.h"
#include "plcBase.h"

/** @file infoPlc.h
	Header which includes classes for the info PLC.
 ************************************************************************/

/** General plan:
	- InfoPLC is created along with System
	- InfoItems are created and updated at appropriate points in the other parts of the code
	- an InfoItem creates several InfoInterfaces with the proper epics_params_structs, and a Record for each, adds these to InfoPLC
	- infoLoad() is called from startup file after tcLoadRecords()
	- infoLoad() generates the dB file from the contents of InfoPLC

	- See infoPLC.cpp, the SimpleInfoItem stuff to see what "InfoItem" is intended to do
 ************************************************************************/

 /** @namespace InfoPlc
	 InfoPlc name space, which has all the classes and functions used for
	 communicating with Info.
	 @brief Namespace for Info communication
  ************************************************************************/
namespace InfoPlc {

class InfoInterface;

/** @defgroup scansettings Constants related to read/write scanning
 ************************************************************************/
/** @{ */

/// default PLC TwinCAT scan rate (100ms)
const int default_scanrate = 100;
/// minimum PLC TwinCAT scan rate (5ms)
const int minimum_scanrate = 5;	
/// maximum PLC TwinCAT scan rate (10s)
const int maximum_scanrate = 10000;

/** @} */

/** This is a statistics value
	@brief Statistic value
 ************************************************************************/
struct stat_value
{
	/// Record
	plc::BaseRecordPtr		rec;
	/// OPC list
	ParseUtil::opc_list		opc;
};

/** This is a list of statistic value
	@brief Statistic list
 ************************************************************************/
typedef std::vector<stat_value> stat_list;

/** This is a base class for a property/value/etc. to keep statistics on
	@brief Base info item
 ************************************************************************/
class BaseInfoItem
{
public:
	/// Default constructor
	BaseInfoItem () {};
	/// Initialize from OPC list
	explicit BaseInfoItem (const ParseUtil::opc_list& opc) : defopc (opc) {};
	/// Destructor
	virtual ~BaseInfoItem() {};
	/// Setup
	/// @param tagname Name of channels
	/// @param plc_alias PLC alias name
	/// @param puser Pointer ot PLC
	virtual bool setup (const std::stringcase& tagname,	
		const std::stringcase& plc_alias, plc::Interface* puser) = 0;
	/// Update
	virtual void update (double val) = 0;
	/// Reset
	virtual void reset() {};

	/// Get OPC list
	const ParseUtil::opc_list& get_opc() const { return defopc; }
	/// Get OPC list
	ParseUtil::opc_list& get_opc() { return defopc; }
	/// get info 
	virtual bool get_info (int idx,
		const std::stringcase& prefix, std::stringcase& name, 
		ParseUtil::process_type_enum& ptype, ParseUtil::opc_list& opc, 
		std::stringcase& type_n, bool& atomic) const;

protected:
	/// statistics list
	stat_list						stats;
	/// OPC list
	ParseUtil::opc_list				defopc;
};

/// Smart pointer to Base info items
typedef std::shared_ptr<BaseInfoItem> BaseInfoItemPtr;
/// List of Base info items
typedef std::vector<BaseInfoItemPtr> BaseInfoList;


/** This is a class to keep simple statistics for a particular property
	@brief Simple info tracker
 ************************************************************************/
class SimpleInfoItem: public BaseInfoItem
{
public:
	/// Default constructor
	SimpleInfoItem () {};
	/// Initialize from OPC list
	explicit SimpleInfoItem (const ParseUtil::opc_list& opc) : BaseInfoItem (opc) {};
	/// Setup
	/// @param tagname Name of channels
	/// @param plc_alias PLC alias name
	/// @param puser Pointer ot PLC
	virtual bool setup (const std::stringcase& tagname,
		const std::stringcase& plc_alias, plc::Interface* puser) override;
	/// Recalculates last, min, max, avg, cnt based on a new value
	virtual void update (double val) override;
	/// Reset
	virtual void reset() override;

protected:
	/// Last
	double							last;
	/// Minimum
	double							min;
	/// Maximum
	double							max;
	/// Average
	double							avg;
	/// Count
	double							cnt;
};


/** This is a class for a histogram
	@brief Histogram statistic
 ************************************************************************/
class HistogramInfoItem	:	public BaseInfoItem
{
public:
	/// Default constructor
	HistogramInfoItem();
	/// Desctructor
	~HistogramInfoItem();
	/// Update
	virtual void					update(double val) override;
	/// Reset
	virtual void					reset() override;
protected:
	/// Upper limit
	double							ulim;
	/// Lower limit
	double							llim;
	/// Number of buckets
	double							nBuckets;
	/// Number of data points
	int								cnt;
	/// Histogram
	std::list<int>					buckets;
};

/** This is a class for keeping the history of a something
	@brief History tracker
 ************************************************************************/
class HistoryInfoItem : public BaseInfoItem
{
public:
	/// Default constructor
	explicit HistoryInfoItem (int length = 10)
	: history_length (length) {};
	/// Constructor from opc list
	explicit HistoryInfoItem (const ParseUtil::opc_list& opc, int length = 10)
		: history_length (length), BaseInfoItem (opc) {};
	/// Desctructor
	~HistoryInfoItem();

	/// Setup
	/// @param tagname Name of channels
	/// @param plc_alias PLC alias name
	/// @param puser Pointer ot PLC
	virtual bool setup (const std::stringcase& tagname,
		const std::stringcase& plc_alias, plc::Interface* puser) override;
	/// Update
	virtual void update(double val) override;
	/// Reset
	virtual void reset() override;
protected:
	/// Length of history to keep
	int								history_length;
	/// History of values
	std::list<double>				history;
};

/** This is a class for timing a process
	@brief Timing tracker
 ************************************************************************/
class TimeInfoItem	:	public SimpleInfoItem
{
public:
	/// Default constructor
	TimeInfoItem();
	/// Desctructor
	~TimeInfoItem();
	/// Start stopwatch
	void							start();
	/// Stop stopwatch
	void							stop();
protected:
	/// Start time
	clock_t							begin;
	/// End time
	clock_t							end;
	/// Elapsed time
	double							elapsed;
};

/** This is a class for a Info interface
	@brief Info interface
 ************************************************************************/
class InfoInterface	:	public plc::Interface
{
public:
	/// Constructor
	InfoInterface(plc::BaseRecord& dval, std::stringcase statname) 
		: Interface(dval), name(statname) {};
	/// Destructor
	~InfoInterface() {};
	/// push data
	virtual bool						push() override {return true; };
	/// pull data
	virtual bool						pull() override {return true; };
protected:
	/// Name of statistics
	std::stringcase						name;
};



/** This is a class for a dummy PLC that tracks connection status, errors, etc.
	@brief Info plc
 ************************************************************************/

class InfoPLC	:	public plc::BasePLC
{
public:
	InfoPLC();
	~InfoPLC();

	/** Iterates over the info list and processes all specified tags.
	@param process Function class
	@param prefix Prefix which is added to all variable names
	@return Number of processes variables
	@brief Process the type tree of a symbol
	*/
	template <class Function>
	int process_info (Function& process, 
		const std::stringcase& prefix = std::stringcase()) const;

	/// get the tag prefix fo rthe info PLC
	const std::stringcase& get_prefix() const { return prefix; }
	/// get the tag prefix fo rthe info PLC
	void set_prefix (const std::stringcase& pre) { prefix = pre; }
protected:
	/// List of statistics values
	BaseInfoList			info_list;

	/** Iterates over the info list and processes all specified tags.
	@param info Info item
	@param process Function class
	@param defopc OPC default list
	@param memloc memory location used to get the EPICS name
	@param prefix Prefix which is added to all variable names
	@return Number of processes variables
	@brief Process the type tree of a symbol
	*/
	template <class Function>
	int process_info (const BaseInfoItem& info,
		Function& process, const ParseUtil::opc_list& defopc, 
		ParseUtil::memory_location& memloc,
		const std::stringcase& prefix = std::stringcase()) const;

	/// Tag prefix
	std::stringcase			prefix;
};

}

#include "infoPlcTemplate.h"