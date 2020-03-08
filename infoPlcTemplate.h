#include "ParseUtilConst.h"

/** @file infoPlcTemplate.h
	Header which includes classes for the info PLC.
 ************************************************************************/
namespace InfoPlc {


/* InfoInterface::get_infodb
 ************************************************************************/
template <class Function>
int InfoInterface::get_infodb(const std::stringcase& prefix, 
	const std::stringcase& plcaddr, Function& proc)
{
	int num = 0;
	for (const auto& rec:dbinfo_list) {
		if (std::get<2>(rec).get_opc_state() != ParseUtil::publish) {
			continue;
		}
		std::stringcase alias = prefix;
		if (!std::get<0>(rec).get_alias().empty()) {
			alias += '.' + std::get<0>(rec).get_alias();
		}
		ParseUtil::variable_name var (
			std::get<0>(rec).get_name(), alias);
		ParseUtil::opc_list opc (std::get<2>(rec));
		process_arg_info arg (var, std::get<1>(rec),
			opc, std::get<3>(rec), std::get<4>(rec), plcaddr);
		if (proc (arg)) {
			++num;
		}
	}
	return num;
}


}
