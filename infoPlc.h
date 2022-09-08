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
using info_update_method = bool (InfoInterface::*)();

/// db info tuple with variable name, process type enum, 
/// opc list, TwinCAT type string, readonly,
/// update enum, and update method
using info_dbrecord_type = std::tuple<ParseUtil::variable_name,
	ParseUtil::process_type_enum,
	ParseUtil::opc_list, 
	std::stringcase, bool, update_enum,
	info_update_method>;

/// List type of db info tuples
using info_dbrecord_list = std::vector<info_dbrecord_type>;


/** This is a class for a Info interface
	@brief Info interface
 ************************************************************************/
class InfoInterface	:	public plc::Interface
{
public:
	/// Constructor
	/// @param dval BaseRecord that this interface is part of
	explicit InfoInterface(plc::BaseRecord& dval) noexcept
		: Interface(dval), update_freq (update_enum::done), 
		readonly(false), info_update (nullptr) {};
	/// Constructor
	/// @param dval BaseRecord that this interface is part of
	/// @param id Short name info symbol
	/// @param name Full name of TCat symbol
	/// @param type Name of TCat data type
	InfoInterface (plc::BaseRecord& dval, const std::stringcase& id, 
		const std::stringcase& name, const std::stringcase& type);

	/// push data
	bool push() noexcept override;
	/// pull data
	bool pull() noexcept override {return true; };
	/// write data
	virtual bool update() noexcept;

	/// Prints TCat symbol value and information
	/// @param fp File to print symbol to
	void printVal(FILE* fp) override;

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
	const char* get_symbol_name() const noexcept override { return tCatName.c_str(); }

protected:
	/// Name of TCat symbol
	std::stringcase		tCatName;
	/// Data type in TCat
	std::stringcase		tCatType;
	/// Update frequency
	update_enum			update_freq;
	/// Readonly flag
	bool				readonly;

	/// pointer to info update method
	info_update_method	info_update;

	/// info update: Name of PLC
	bool info_update_name () noexcept;
	/// info update: Alias name
	bool info_update_alias() noexcept;
	/// info update: Running state of PLC
	bool info_update_active() noexcept;
	/// info update: AMS state of PLC
	bool info_update_state() noexcept;
	/// info update: AMS state of PLC
	bool info_update_statestr() noexcept;
	/// info update: PLC time stamp
	bool info_update_timestamp_str() noexcept;
	/// info update: PLC time stamp (local time)
	bool info_update_timestamp_local() noexcept;
	/// info update: Year of PLC time stamp
	bool info_update_timestamp_year() noexcept;
	/// info update: Month of PLC time stamp
	bool info_update_timestamp_month() noexcept;
	/// info update: Day of PLC time stamp
	bool info_update_timestamp_day() noexcept;
	/// info update: Hour of PLC time stamp
	bool info_update_timestamp_hour() noexcept;
	/// info update: Minute of PLC time stamp
	bool info_update_timestamp_min() noexcept;
	/// info update: Second of PLC time stamp
	bool info_update_timestamp_sec() noexcept;
	/// info update: Period of read scanner in ms
	bool info_update_rate_read() noexcept;
	/// info update: Period of write scanner in ms
	bool info_update_rate_write() noexcept;
	/// info update: Period of update scanner in ms
	bool info_update_rate_update() noexcept;
	/// info update: Number of EPICS records
	bool info_update_records_num() noexcept;
	/// info update: Name of typ file
	bool info_update_tpy_filename() noexcept;
	/// info update: Validity of tpy file
	bool info_update_tpy_valid() noexcept;
	/// info update: Modifcation time of tpy file
	bool info_update_tpy_time_str() noexcept;
	/// info update: Year of tpy file time
	bool info_update_tpy_time_year() noexcept;
	/// info update: Month of tpy file time
	bool info_update_tpy_time_month() noexcept;
	/// info update: Day of tpy file time
	bool info_update_tpy_time_day() noexcept;
	/// info update: Hour of tpy file time
	bool info_update_tpy_time_hour() noexcept;
	/// info update: Minute of tpy file time
	bool info_update_tpy_time_min() noexcept;
	/// info update: Second of tpy file time
	bool info_update_tpy_time_sec() noexcept;
	/// info update: ADS library version
	bool info_update_ads_version() noexcept;
	/// info update: ADS library revision
	bool info_update_ads_revision() noexcept;
	/// info update: ADS library build
	bool info_update_ads_build() noexcept;
	/// info update: ADS/AMS port of PLC
	bool info_update_ads_port() noexcept;
	/// info update: ADS/AMS address of PLC
	bool info_update_ads_netid_str() noexcept;
	/// info update: ADS/AMS address b0
	bool info_update_ads_netid_b0() noexcept;
	/// info update: ADS/AMS address b1
	bool info_update_ads_netid_b1() noexcept;
	/// info update: ADS/AMS address b2
	bool info_update_ads_netid_b2() noexcept;
	/// info update: ADS/AMS address b3
	bool info_update_ads_netid_b3() noexcept;
	/// info update: ADS/AMS address b4
	bool info_update_ads_netid_b4() noexcept;
	/// info update: ADS/AMS address b5
	bool info_update_ads_netid_b5() noexcept;
	/// info update: SVN local modifications
	bool info_update_svn_local() noexcept;
	/// info update: SVN revision, if fully committed
	bool info_update_svn_revision() noexcept;
	/// info update: SVN compile time
	bool info_update_svn_time() noexcept;
	/// info update: Size of low priority callback queue
	bool info_update_callback_queue0_size() noexcept;
	/// info update: Used entries in low priority callback queue
	bool info_update_callback_queue0_used() noexcept;
	/// info update: Maximum used entries (high watermark) in low priority callback queue
	bool info_update_callback_queue0_max() noexcept;
	/// info update: Overflows in low priority callback queue
	bool info_update_callback_queue0_overflow() noexcept;
	/// info update: Free entries low priority callback queue
	bool info_update_callback_queue0_free() noexcept;
	/// info update: Usage percentage of low priority callback queue
	bool info_update_callback_queue0_percent() noexcept;
	/// info update: Maximum percentage of low priority callback queue
	bool info_update_callback_queue0_max_prcnt() noexcept;
	/// info update: Size of medium priority callback queue
	bool info_update_callback_queue1_size() noexcept;
	/// info update: Used entries in medium priority callback queue
	bool info_update_callback_queue1_used() noexcept;
	/// info update: Maximum used entries (high watermark) in medium priority callback queue
	bool info_update_callback_queue1_max() noexcept;
	/// info update: Overflows in medium priority callback queue
	bool info_update_callback_queue1_overflow() noexcept;
	/// info update: Free entries medium priority callback queue
	bool info_update_callback_queue1_free() noexcept;
	/// info update: Usage percentage of medium priority callback queue
	bool info_update_callback_queue1_percent() noexcept;
	/// info update: Maximum percentage of medium priority callback queue
	bool info_update_callback_queue1_max_prcnt() noexcept;
	/// info update: Size of high priority callback queue
	bool info_update_callback_queue2_size() noexcept;
	/// info update: Used entries in high priority callback queue
	bool info_update_callback_queue2_used() noexcept;
	/// info update: Maximum used entries (high watermark) in high priority callback queue
	bool info_update_callback_queue2_max() noexcept;
	/// info update: Overflows in high priority callback queue
	bool info_update_callback_queue2_overflow() noexcept;
	/// info update: Free entries high priority callback queue
	bool info_update_callback_queue2_free() noexcept;
	/// info update: Usage percentage of high priority callback queue
	bool info_update_callback_queue2_percent() noexcept;
	/// info update: Maximum percentage of high priority callback queue
	bool info_update_callback_queue2_max_prcnt() noexcept;
	/// info update: reset maximum values of callback buffer queues
	bool info_update_callback_queue_reset_max() noexcept;

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
	std::stringcase get_full() const override;

protected:
	/// TwinCAT PLC addres
	const std::stringcase& tcplc_addr;
};

/** @} */

}

#include "infoPlcTemplate.h"