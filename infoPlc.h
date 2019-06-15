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


/** @defgroup infoplc Classes and functions related to the info interface
 ************************************************************************/
/** @{ */

/// db info tuple with variable name, type name, process type enum, 
typedef std::tuple<ParseUtil::variable_name, 
	ParseUtil::process_type_enum,
	ParseUtil::opc_list, 
	std::stringcase, bool> info_dbrecord_type;

/// List type of db info tuples
typedef std::vector<info_dbrecord_type> info_dbrecord_list;


/** This is a class for a Info interface
	@brief Info interface
 ************************************************************************/
class InfoInterface	:	public plc::Interface
{
public:
	/// Constructor
	explicit InfoInterface(plc::BaseRecord& dval)
		: Interface(dval) {};
	/// Constructor
	/// @param dval BaseRecord that this interface is part of
	/// @param name Name of TCat symbol
	/// @param type Name of TCat data type
	/// @param isStruct True = this symbol is a structure in TCat
	/// @param isEnum True = this symbol is an enum in TCat
	InfoInterface (plc::BaseRecord& dval, const std::stringcase& name,
		const std::stringcase& type, bool isStruct, bool isEnum);
	/// Deconstructor
	~InfoInterface() {};

	/// Constructor
	InfoInterface (plc::BaseRecord& dval, const std::stringcase& name, 
		const std::stringcase& tname)
		: Interface(dval), tCatName (name), tCatType (tname) {};
	/// push data
	virtual bool						push() override {return true; };
	/// pull data
	virtual bool						pull() override {return true; };

	/// Prints TCat symbol value and information
	/// @param fp File to print symbol to
	virtual void printVal(FILE* fp);

	/// Porcess the EPICS datbacse for the info records
	/// @param prefix Prefix to channel names in the info datbase
	/// @param proc database processing
	/// @return Returns an EPICS database string describing the info records
	template <class Function>
	static int get_infodb(const std::stringcase& prefix, 
		const std::stringcase& plcaddr, Function& proc);

protected:
	/// Name of TCat symbol
	std::stringcase		tCatName;
	/// Data type in TCat
	std::stringcase		tCatType;

	/// List of db info records
	static info_dbrecord_list dbinfo_list;
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
	/// #param plcaddr TwinCAT PLC address
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