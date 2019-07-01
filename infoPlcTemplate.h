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
		if (std::get<ParseUtil::opc_list>(rec).get_opc_state() != publish) {
			continue;
		}
		std::stringcase alias = prefix;
		if (!std::get<ParseUtil::variable_name>(rec).get_alias().empty()) {
			alias += '.' + std::get<ParseUtil::variable_name>(rec).get_alias();
		}
		ParseUtil::variable_name var (
			std::get<ParseUtil::variable_name>(rec).get_name(), alias);
		ParseUtil::opc_list opc (std::get<ParseUtil::opc_list>(rec));
		process_arg_info arg (var, std::get< ParseUtil::process_type_enum>(rec),
			opc, std::get<std::stringcase>(rec), std::get<bool>(rec), plcaddr);
		if (proc (arg)) {
			++num;
		}
	}
	return num;
}


}
