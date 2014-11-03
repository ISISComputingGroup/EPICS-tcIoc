#pragma once
#include "stdafx.h"
#include "devTc.h"

/** @file devInfo.h
	Header which includes classes for Info device support. 
 ************************************************************************/

/** @namespace DevTc
	DevTc name space 
 ************************************************************************/

namespace DevInfo {

/** Initialization function that matches an EPICS record with an internal
	Info record entry
	@param pEpicsRecord Pointer to EPICS record
	@param pRecord Pointer to a base record
	@return true if successful
	@brief linkInfoRecord
 ************************************************************************/
bool linkInfoRecord (dbCommon* pEpicsRecord, plc::BaseRecordPtr& pRecord);

/// Regex for indentifying TwinCAT records
const std::regex info_regex (
	"((info)://((\\b([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.?)+:(8[0-9][0-9]))/)(\\d{1,9})/(\\d{1,9}):(\\d{1,9})");

/** This is a class for an EPICS Interface
    @brief epics interface class.
 ************************************************************************/
class register_info_devsup
{
protected:
	/// Default constructor; adds a linkInfoRecord entry
	register_info_devsup() {
		DevTc::register_devsup::add (info_regex, linkInfoRecord); }
	static register_info_devsup the_register_info_devsup;
};

}