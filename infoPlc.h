#pragma once
#include "stdafx.h"
#include "ParseUtil.h"
#include "plcBase.h"

/** @file infoPlc.h
	Header which includes classes for the info PLC.
 ************************************************************************/


 /** @namespace InfoPlc
	 InfoPlc name space, which has all the classes and functions used for
	 communicating with Info.
	 @brief Namespace for Info communication
  ************************************************************************/
namespace InfoPlc {


/** @defgroup infoplc Info interface classes and functions
 ************************************************************************/
/** @{ */


class InfoInterface;

/// Update frequency type
enum class update_enum {
	/// Repeat foreveer
	forever,
	/// once
	once,
	/// done
	done
};

/// Pointer to info update method
typedef bool (InfoInterface::*info_update_method)();

/// db info tuple with variable name, process type enum, 
/// opc list, TwinCAT type string, readonly,
/// update enum, and update method
typedef std::tuple<ParseUtil::variable_name, 
	ParseUtil::process_type_enum,
	ParseUtil::opc_list, 
	std::stringcase, bool, update_enum,
	info_update_method> info_dbrecord_type;

/// List type of db info tuples
typedef std::vector<info_dbrecord_type> info_dbrecord_list;


/** This is a class for a Info interface
	@brief Info interface
 ************************************************************************/
class InfoInterface	:	public plc::Interface
{
public:
	/// Constructor
	/// @param dval BaseRecord that this interface is part of
	explicit InfoInterface(plc::BaseRecord& dval)
		: Interface(dval), update_freq (update_enum::done), 
		info_update (nullptr) {};
	/// Constructor
	/// @param dval BaseRecord that this interface is part of
	/// @param id Short name info symbol
	/// @param name Full name of TCat symbol
	/// @param type Name of TCat data type
	InfoInterface (plc::BaseRecord& dval, const std::stringcase& id, 
		const std::stringcase& name, const std::stringcase& type);
	/// Deconstructor
	~InfoInterface() {};

	/// push data
	virtual bool push() override {return true; };
	/// pull data
	virtual bool pull() override {return true; };
	/// write data
	virtual bool update();

	/// Prints TCat symbol value and information
	/// @param fp File to print symbol to
	virtual void printVal(FILE* fp);

	/// Processes all info records
	/// @param prefix Prefix to channel names in the info datbase
	/// @param plcaddr PLC associated with the info record
	/// @param proc database processing class
	/// @return Returns Number of info records processed
	template <class Function>
	static int get_infodb(const std::stringcase& prefix, 
		const std::stringcase& plcaddr, Function& proc);

	/// Get symbol name
	/// @return Symbol name associated with this interface
	virtual const char* get_symbol_name() const { return tCatName.c_str(); }

protected:
	/// Name of TCat symbol
	std::stringcase		tCatName;
	/// Data type in TCat
	std::stringcase		tCatType;
	/// Update frequency
	update_enum			update_freq;

	/// pointer to info update method
	info_update_method	info_update;

	/// info update: Name of PLC
	bool info_update_name ();
	/// info update: Alias name
	bool info_update_alias();
	/// info update: Running state of PLC
	bool info_update_active();
	/// info update: AMS state of PLC
	bool info_update_state();
	/// info update: AMS state of PLC
	bool info_update_statestr();
	/// info update: PLC time stamp
	bool info_update_timestamp_str();
	/// info update: PLC time stamp (local time)
	bool info_update_timestamp_local();
	/// info update: Year of PLC time stamp
	bool info_update_timestamp_year();
	/// info update: Month of PLC time stamp
	bool info_update_timestamp_month();
	/// info update: Day of PLC time stamp
	bool info_update_timestamp_day();
	/// info update: Hour of PLC time stamp
	bool info_update_timestamp_hour();
	/// info update: Minute of PLC time stamp
	bool info_update_timestamp_min();
	/// info update: Second of PLC time stamp
	bool info_update_timestamp_sec();
	/// info update: Period of read scanner in ms
	bool info_update_rate_read();
	/// info update: Period of write scanner in ms
	bool info_update_rate_write();
	/// info update: Period of update scanner in ms
	bool info_update_rate_update();
	/// info update: Number of EPICS records
	bool info_update_records_num();
	/// info update: Name of typ file
	bool info_update_tpy_filename();
	/// info update: Validity of tpy file
	bool info_update_tpy_valid();
	/// info update: Modifcation time of tpy file
	bool info_update_tpy_time_str();
	/// info update: Year of tpy file time
	bool info_update_tpy_time_year();
	/// info update: Month of tpy file time
	bool info_update_tpy_time_month();
	/// info update: Day of tpy file time
	bool info_update_tpy_time_day();
	/// info update: Hour of tpy file time
	bool info_update_tpy_time_hour();
	/// info update: Minute of tpy file time
	bool info_update_tpy_time_min();
	/// info update: Second of tpy file time
	bool info_update_tpy_time_sec();
	/// info update: ADS library version
	bool info_update_ads_version();
	/// info update: ADS library revision
	bool info_update_ads_revision();
	/// info update: ADS library build
	bool info_update_ads_build();
	/// info update: ADS/AMS port of PLC
	bool info_update_ads_port();
	/// info update: ADS/AMS address of PLC
	bool info_update_ads_netid_str();
	/// info update: ADS/AMS address b0
	bool info_update_ads_netid_b0();
	/// info update: ADS/AMS address b1
	bool info_update_ads_netid_b1();
	/// info update: ADS/AMS address b2
	bool info_update_ads_netid_b2();
	/// info update: ADS/AMS address b3
	bool info_update_ads_netid_b3();
	/// info update: ADS/AMS address b4
	bool info_update_ads_netid_b4();
	/// info update: ADS/AMS address b5
	bool info_update_ads_netid_b5();
	/// info update: SVN local modifications
	bool info_update_svn_local();
	/// info update: SVN revision, if fully committed
	bool info_update_svn_revision();
	/// info update: SVN compile time
	bool info_update_svn_time();
	/// info update: Size of low priority callback queue
	bool info_update_callback_queue0_size();
	/// info update: Used entries in low priority callback queue
	bool info_update_callback_queue0_used();
	/// info update: Maximum used entries in low priority callback queue
	bool info_update_callback_queue0_max();
	/// info update: Free entries low priority callback queue
	bool info_update_callback_queue0_free();
	/// info update: Usage percentage of low priority callback queue
	bool info_update_callback_queue0_percent();
	/// info update: Maximum percentage of low priority callback queue
	bool info_update_callback_queue0_mprcnt();
	/// info update: Size of medium priority callback queue
	bool info_update_callback_queue1_size();
	/// info update: Used entries in medium priority callback queue
	bool info_update_callback_queue1_used();
	/// info update: Maximum used entries in medium priority callback queue
	bool info_update_callback_queue1_max();
	/// info update: Free entries medium priority callback queue
	bool info_update_callback_queue1_free();
	/// info update: Usage percentage of medium priority callback queue
	bool info_update_callback_queue1_percent();
	/// info update: Maximum percentage of medium priority callback queue
	bool info_update_callback_queue1_mprcnt();
	/// info update: Size of high priority callback queue
	bool info_update_callback_queue2_size();
	/// info update: Used entries in high priority callback queue
	bool info_update_callback_queue2_used();
	/// info update: Maximum used entries in high priority callback queue
	bool info_update_callback_queue2_max();
	/// info update: Free entries high priority callback queue
	bool info_update_callback_queue2_free();
	/// info update: Usage percentage of high priority callback queue
	bool info_update_callback_queue2_percent();
	/// info update: Maximum percentage of high priority callback queue
	bool info_update_callback_queue2_mprcnt();

	/// List of db info records
	static const info_dbrecord_list dbinfo_list;
};

/** Argument which is passed to the name/tag processing function.
	@brief Arguments for processing
************************************************************************/
class process_arg_info : public ParseUtil::process_arg
{
public:
	/// Constructor
	/// @param vname Variable name
	/// @param pt Process type 
	/// @param o OPC list
	/// @param tname Type name
	/// @param at Atomic type
	/// @param plcaddr TwinCAT PLC address
	process_arg_info (
		const ParseUtil::variable_name& vname, ParseUtil::process_type_enum pt,
		const ParseUtil::opc_list& o, const std::stringcase& tname, bool at,
		const std::stringcase& plcaddr)
		: process_arg (vname, pt, o, tname, at), tcplc_addr(plcaddr) {}

	/// Gets a string representation of a PLC & memory location
	/// @return string with format "prefixigroup/ioffset:size", empty on error
	virtual std::stringcase get_full() const;

protected:
	/// TwinCAT PLC addres
	const std::stringcase& tcplc_addr;
};

/** @} */

}

#include "infoPlcTemplate.h"