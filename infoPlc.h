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
	virtual bool pull() override;

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

	/// info update: name
	bool info_update_name ();
	/// info update: alias
	bool info_update_alias();
	/// info update: active
	bool info_update_active();
	/// info update: state
	bool info_update_state();
	/// info update: statestr
	bool info_update_statestr();
	/// info update: timestamp_str
	bool info_update_timestamp_str();
	/// info update: timestamp_year
	bool info_update_timestamp_year();
	/// info update: timestamp_month
	bool info_update_timestamp_month();
	/// info update: timestamp_day
	bool info_update_timestamp_day();
	/// info update: timestamp_hour
	bool info_update_timestamp_hour();
	/// info update: timestamp_min
	bool info_update_timestamp_min();
	/// info update: timestamp_sec
	bool info_update_timestamp_sec();
	/// info update: rate_read
	bool info_update_rate_read();
	/// info update: rate_write
	bool info_update_rate_write();
	/// info update: rate_update
	bool info_update_rate_update();
	/// info update: records_num
	bool info_update_records_num();
	/// info update: tpy_filename
	bool info_update_tpy_filename();
	/// info update: tpy_valid
	bool info_update_tpy_valid();
	/// info update: tpy_time_str
	bool info_update_tpy_time_str();
	/// info update: tpy_time_year
	bool info_update_tpy_time_year();
	/// info update: tpy_time_month
	bool info_update_tpy_time_month();
	/// info update: tpy_time_day
	bool info_update_tpy_time_day();
	/// info update: tpy_time_hour
	bool info_update_tpy_time_hour();
	/// info update: tpy_time_min
	bool info_update_tpy_time_min();
	/// info update: tpy_time_sec
	bool info_update_tpy_time_sec();
	/// info update: ads_version
	bool info_update_ads_version();
	/// info update: ads_revision
	bool info_update_ads_revision();
	/// info update: ads_build
	bool info_update_ads_build();
	/// info update: ads_port
	bool info_update_ads_port();
	/// info update: ads_netid_str
	bool info_update_ads_netid_str();
	/// info update: ads_netid_b0
	bool info_update_ads_netid_b0();
	/// info update: ads_netid_b1
	bool info_update_ads_netid_b1();
	/// info update: ads_netid_b2
	bool info_update_ads_netid_b2();
	/// info update: ads_netid_b3
	bool info_update_ads_netid_b3();
	/// info update: ads_netid_b4
	bool info_update_ads_netid_b4();
	/// info update: ads_netid_b5
	bool info_update_ads_netid_b5();
	/// info update: svn_local
	bool info_update_svn_local();
	/// info update: svn_revision
	bool info_update_svn_revision();
	/// info update: svn_time
	bool info_update_svn_time();
	/// info update: callback_queue_size
	bool info_update_callback_queue_size();
	/// info update: callback_queue_used
	bool info_update_callback_queue_used();
	/// info update: callback_queue_free
	bool info_update_callback_queue_free();

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