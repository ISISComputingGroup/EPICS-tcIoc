#include "infoPlc.h"

using namespace std;
using namespace ParseUtil;
using namespace plc;

namespace InfoPlc {

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