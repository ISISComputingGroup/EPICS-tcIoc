#include "ParseUtilConst.h"

/** @file infoPlcTemplate.h
	Header which includes classes for the info PLC.
 ************************************************************************/
namespace InfoPlc {

template <class Function>
int InfoPLC::process_info (Function& process, 
				  const std::stringcase& prefix) const
{
	ParseUtil::opc_list defopc;
	defopc.set_opc_state (ParseUtil::publish);
	defopc.add (ParseUtil::property_el (OPC_PROP_PLCNAME, "info://0.0.0.0.0.0:1/"));
	ParseUtil::memory_location memloc (1, 0, 0);
	int num = 0;
	for (auto i : info_list) {
		num += process_info (*i, process, defopc, memloc, prefix);
		memloc.set_ioffset (memloc.get_ioffset() + 1);
	}
	return num;
}

/* InfoPLC::process_info
 ************************************************************************/
template <class Function>
int InfoPLC::process_info (const BaseInfoItem& info,
	Function& process, const ParseUtil::opc_list& defopc, 
	ParseUtil::memory_location& memloc,
	const std::stringcase& prefix) const 
{
	std::stringcase name;
	std::stringcase type_n;
	ParseUtil::process_type_enum ptype;
	ParseUtil::opc_list opc (defopc);
	bool atomic;
	int num = 0;
	int idx = 0;
	while (info.get_info (idx, prefix, name, ptype, opc, type_n, atomic)) {
		memloc.set_bytesize (idx);
		++idx;
		process_arg arg (memloc, ParseUtil::variable_name(name), ptype, 
			opc, type_n, atomic);
		if (process (arg)) ++num;
		opc = defopc;
	}
	return num;
}


}
