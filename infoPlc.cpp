#include "infoPlc.h"

using namespace std;
using namespace ParseUtil;
using namespace plc;

/** @file InfoPlc.cpp
	Defines methods for the info PLC.
 ************************************************************************/

namespace InfoPlc {

/// Database for PLC info records
const char* const infodb = R"++(
record(stringin,"${PREFIX}.name") {
	field(DESC,"PCL name")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/name")
	field(TSE,"-2")
	field(PINI,"0")
}
record(stringin,"${PREFIX}.alias") {
	field(DESC,"Tpy filename")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/alias")
	field(TSE,"-2")
	field(PINI,"0")
}
record(bi,"${PREFIX}.active") {
	field(DESC,"Running state of PLC")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/active")
	field(TSE,"-2")
	field(PINI,"0")
	field(ONAM,"ONLINE")
	field(ZNAM,"OFFLINE")
}
record(longin,"${PREFIX}.state") {
	field(DESC,"AMS state of PLC")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/state")
	field(TSE,"-2")
	field(PINI,"0")
}
record(stringin,"${PREFIX}.statestr") {
	field(DESC,"AMS state of PLC")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/statestr")
	field(TSE,"-2")
	field(PINI,"0")
}
record(stringin,"${PREFIX}.timestamp.str") {
	field(DESC,"PLC time stamp")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/timestamp.str")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.timestamp.year") {
	field(DESC,"PLC time stamp")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/timestamp.year")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.timestamp.month") {
	field(DESC,"PLC time stamp")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/timestamp.month")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.timestamp.day") {
	field(DESC,"PLC time stamp")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/timestamp.day")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.timestamp.hour") {
	field(DESC,"PLC time stamp")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/timestamp.hour")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.timestamp.min") {
	field(DESC,"PLC time stamp")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/timestamp.min")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.timestamp.sec") {
	field(DESC,"Timestamp")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/timestamp.sec")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.rate.read") {
	field(DESC,"PLC read rate in ms")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/rate.read")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.rate.write") {
	field(DESC,"PLC read rate in ms")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/rate.write")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.rate.update") {
	field(DESC,"PLC update rate in ms")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/rate.update")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.records.num") {
	field(DESC,"Number of channels")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/records.num")
	field(TSE,"-2")
	field(PINI,"0")
}
record(stringin,"${PREFIX}.tpy.filename") {
	field(DESC,"Tpy filename")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/tpy.filename")
	field(TSE,"-2")
	field(PINI,"0")
}
record(bi,"${PREFIX}.typ.valid") {
	field(DESC,"Tpy file valid")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/typ.valid")
	field(TSE,"-2")
	field(PINI,"0")
	field(ONAM,"VALID")
	field(ZNAM,"INVALID")
}
record(stringin,"${PREFIX}.tpy.mod.str") {
	field(DESC,"Tpy file modification time")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/tpy.mod.str")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.tpy.mod.year") {
	field(DESC,"Tpy file modification time")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/tpy.mod.year")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.tpy.mod.month") {
	field(DESC,"Tpy file modification time")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/tpy.mod.month")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.tpy.mod.day") {
	field(DESC,"Tpy file modification time")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/tpy.mod.day")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.tpy.mod.hour") {
	field(DESC,"Tpy file modification time")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/tpy.mod.hour")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.tpy.mod.min") {
	field(DESC,"Tpy file modification time")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/tpy.mod.min")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.tpy.mod.sec") {
	field(DESC,"Tpy file modification time")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/tpy.mod.sec")
	field(TSE,"-2")
	field(PINI,"0")
}
record(stringin,"${PREFIX}.addr.netid.str") {
	field(DESC,"AMS Net ID address")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/addr.netid.str")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.addr.port") {
	field(DESC,"AMS port address")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/addr.port")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.addr.netid.b[0]") {
	field(DESC,"AMS Net ID address")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/addr.netid.b[0]")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.addr.netid.b[1]") {
	field(DESC,"AMS Net ID address")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/addr.netid.b[1]")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.addr.netid.b[2]") {
	field(DESC,"AMS Net ID address")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/addr.netid.b[2]")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.addr.netid.b[3]") {
	field(DESC,"AMS Net ID address")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/addr.netid.b[3]")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.addr.netid.b[4]") {
	field(DESC,"AMS Net ID address")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/addr.netid.b[4]")
	field(TSE,"-2")
	field(PINI,"0")
}
record(longin,"${PREFIX}.addr.netid.b[5]") {
	field(DESC,"AMS Net ID address")
	field(SCAN,"I/O Intr")
	field(DTYP,"tcat")
	field(INP,"@${PLCADR}/info/addr.netid.b[5]")
	field(TSE,"-2")
	field(PINI,"0")
}
)++";

const char* const infodb_prefix = "PREFIX";
const char* const infodb_tcaddr = "PLCADR";

/* InfoInterface::get_infodb
 ************************************************************************/
std::stringcase InfoInterface::get_infodb (const std::stringcase& prefix,
	const std::stringcase& tcaddr)
{
	std::stringcase db (infodb);
	// create rules table 
	ParseUtil::replacement_table rtable;
	rtable[infodb_prefix] = prefix;
	trim_space (rtable[infodb_prefix]);
	rtable[infodb_tcaddr] = tcaddr;
	trim_space (rtable[infodb_tcaddr]);
	// remove trailing slash in tc addr
	if (!rtable[infodb_tcaddr].empty() &&
		(*rtable[infodb_tcaddr].crbegin() == '/')) {
		rtable[infodb_tcaddr].erase(rtable[infodb_tcaddr].size()-1, 1);
	}
	// apply rules
	ParseUtil::replacement_rules rrules (rtable, false);
	return rrules.apply_replacement_rules(db);
}

/** BaseInfoItem::get_info
 ************************************************************************/
bool BaseInfoItem::get_info (int idx,
		const std::stringcase& prefix, std::stringcase& name, 
		ParseUtil::process_type_enum& ptype, ParseUtil::opc_list& opc, 
		std::stringcase& type_n, bool& atomic) const
{
	if ((idx < 0) || (idx >= (int)stats.size())) {
		return false;
	}
	if (!stats[idx].rec.get()) {
		return false;
	}
	opc.add (defopc);
	name = prefix + stats[idx].rec->get_name();
	opc.add (stats[idx].opc);
	atomic = true;
	switch (stats[idx].rec->get_data().get_data_type()) {
	case dtBool:
		type_n = "BOOL";
		ptype = pt_bool;
		break;
	case dtInt8:
		type_n = "SINT";
		ptype = pt_int;
		break;
	case dtUInt8:
		type_n = "USINT";
		ptype = pt_int;
		break;
	case dtInt16:
		type_n = "INT";
		ptype = pt_int;
		break;
	case dtUInt16:
		type_n = "UINT";
		ptype = pt_int;
		break;
	case dtInt32:
		type_n = "DINT";
		ptype = pt_int;
		break;
	case dtUInt32:
		type_n = "UDINT";
		ptype = pt_int;
		break;
	case dtInt64:
		type_n = "LINT";
		ptype = pt_int;
		break;
	case dtUInt64:
		type_n = "ULINT";
		ptype = pt_int;
		break;
	case dtFloat:
		type_n = "REAL";
		ptype = pt_real;
		break;
	case dtDouble:
		type_n = "LREAL";
		ptype = pt_real;
		break;
	case dtString:
		type_n = "STRING";
		ptype = pt_string;
		break;
	case dtWString:
		type_n = "WSTRING";
		ptype = pt_string;
		break;
	case dtBinary:
		type_n = "STRUCT";
		ptype = pt_binary;
		break;
	default:
		return false;
	}
	return true;
}


/** SimpleInfoItem
 ************************************************************************/
bool SimpleInfoItem::setup (const std::stringcase& tagname,	
	const std::stringcase& plc_alias, plc::Interface* puser)
{
	return false;
}

void SimpleInfoItem::update(double val)
{
	last = val;
	if (val < min) min = val;
	if (val > max) max = val;
	cnt++;
	avg = (avg*(cnt-1) + val)/cnt;
}

void SimpleInfoItem::reset()
{
	min = last;
	max = last;
	avg = last;
	cnt = 1;
}

/** HistoryInfoItem
 ************************************************************************/
bool HistoryInfoItem::setup (const std::stringcase& tagname,
		const std::stringcase& plc_alias, plc::Interface* puser) 
{
	return false;
}

void HistoryInfoItem::update(double val)
{
	if (history.size() == history_length) {
		history.pop_front();
	}
	history.push_back(val);
}

void HistoryInfoItem::reset()
{
	history.clear();
}

/** TimeInfoItem
 ************************************************************************/

void TimeInfoItem::start()
{
	begin = clock();
}

void TimeInfoItem::stop()
{
	end = clock();
	elapsed = (end-begin)/CLOCKS_PER_SEC;
	if (elapsed <= 0) {
	
	}
	update(elapsed);
}

}