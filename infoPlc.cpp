#include <epicsTime.h>
#include "infoPlc.h"
#include "tcComms.h"
#include "svn_version.h"

using namespace std;
using namespace ParseUtil;
using namespace plc;

/// @cond Doxygen_Suppress
extern "C" {
	int get_callback_queue_size(int pri);
	int get_callback_queue_used(int pri);
	int get_callback_queue_free(int pri);
}
/// @endcond

/** @file InfoPlc.cpp
	Defines methods for the info PLC.
 ************************************************************************/

namespace InfoPlc {

static bool debug = false;


/// List of of db info tuples
const info_dbrecord_list InfoInterface::dbinfo_list({
info_dbrecord_type(
	variable_name(""),
	process_type_enum::pt_binary,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "3"),
		property_el(OPC_PROP_DESC, "PLC Info structure")
	})),
	"InfoStruct", false, update_enum::once,
	&InfoInterface::info_update_name),
info_dbrecord_type (
	variable_name("name"),
	process_type_enum::pt_string,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Name of PLC")
	})),
	"STRING", true, update_enum::once,
	&InfoInterface::info_update_name),
info_dbrecord_type(
	variable_name("alias"),
	process_type_enum::pt_string,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Alias name")
		})),
	"STRING", true, update_enum::once,
	&InfoInterface::info_update_alias),
info_dbrecord_type(
	variable_name("active"),
	process_type_enum::pt_bool,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Running state of PLC"),
		property_el(OPC_PROP_CLOSE, "ONLINE"),
		property_el(OPC_PROP_OPEN, "OFFLINE")
		})),
	"BOOL", true, update_enum::forever,
	&InfoInterface::info_update_active),
info_dbrecord_type (
	variable_name("state"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "AMS state of PLC")
		})),
	"DINT", true, update_enum::forever,
	&InfoInterface::info_update_state),
info_dbrecord_type(
	variable_name("statestr"),
	process_type_enum::pt_string,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "AMS state of PLC")
		})),
	"STRING", true, update_enum::forever,
	&InfoInterface::info_update_statestr),
info_dbrecord_type(
	variable_name("timestamp.str"),
	process_type_enum::pt_string,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "PLC time stamp")
		})),
	"STRING", true, update_enum::forever,
	&InfoInterface::info_update_timestamp_str),
info_dbrecord_type(
	variable_name("timestamp.year"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Year of PLC time stamp")
		})),
	"INT", true, update_enum::forever,
	&InfoInterface::info_update_timestamp_year),
info_dbrecord_type(
	variable_name("timestamp.month"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Month of PLC time stamp")
		})),
	"INT", true, update_enum::forever,
	&InfoInterface::info_update_timestamp_month),
info_dbrecord_type(
	variable_name("timestamp.day"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Day of PLC time stamp")
		})),
	"INT", true, update_enum::forever,
	&InfoInterface::info_update_timestamp_day),
info_dbrecord_type(
	variable_name("timestamp.hour"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Hour of PLC time stamp")
		})),
	"INT", true, update_enum::forever,
	&InfoInterface::info_update_timestamp_hour),
info_dbrecord_type(
	variable_name("timestamp.min"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Minute of PLC time stamp")
		})),
	"INT", true, update_enum::forever,
	&InfoInterface::info_update_timestamp_min),
info_dbrecord_type(
	variable_name("timestamp.sec"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Second of PLC time stamp")
		})),
	"INT", true, update_enum::forever,
	&InfoInterface::info_update_timestamp_sec),
info_dbrecord_type(
	variable_name("rate.read"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Period of read scanner in ms"),
		property_el(OPC_PROP_UNIT, "ms")
		})),
	"DINT", true, update_enum::once,
	&InfoInterface::info_update_rate_read),
info_dbrecord_type(
	variable_name("rate.write"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Period of write scanner in ms"),
		property_el(OPC_PROP_UNIT, "ms")
		})),
	"DINT", true, update_enum::once,
	&InfoInterface::info_update_rate_write),
info_dbrecord_type(
	variable_name("rate.update"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Period of update scanner in ms"),
		property_el(OPC_PROP_UNIT, "ms")
		})),
	"DINT", true, update_enum::once,
	&InfoInterface::info_update_rate_update),
info_dbrecord_type(
	variable_name("records.num"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Number of records")
		})),
	"DINT", true, update_enum::once,
	&InfoInterface::info_update_records_num),
info_dbrecord_type (
	variable_name("tpy.filename"),
	process_type_enum::pt_string,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Name of typ file")
	})),
	"STRING", true, update_enum::once,
	&InfoInterface::info_update_tpy_filename),
info_dbrecord_type(
	variable_name("tpy.valid"),
	process_type_enum::pt_bool,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Validity of tpy file"),
		property_el(OPC_PROP_CLOSE, "VALID"),
		property_el(OPC_PROP_OPEN, "INVALID")
		})),
	"BOOL", true, update_enum::forever,
	&InfoInterface::info_update_tpy_valid),
info_dbrecord_type(
	variable_name("tpy.time.str"),
	process_type_enum::pt_string,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Modifcation time of tpy file")
		})),
	"STRING", true, update_enum::forever,
	&InfoInterface::info_update_tpy_time_str),
info_dbrecord_type(
	variable_name("tpy.time.year"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Year of tpy file time")
		})),
	"INT", true, update_enum::forever,
	&InfoInterface::info_update_tpy_time_year),
info_dbrecord_type(
	variable_name("tpy.time.month"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Month of tpy file time")
		})),
	"INT", true, update_enum::forever,
	&InfoInterface::info_update_tpy_time_month),
info_dbrecord_type(
	variable_name("tpy.time.day"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Day of tpy file time")
		})),
	"INT", true, update_enum::forever,
	&InfoInterface::info_update_tpy_time_day),
info_dbrecord_type(
	variable_name("tpy.time.hour"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Hour of tpy file time")
		})),
	"INT", true, update_enum::forever,
	&InfoInterface::info_update_tpy_time_hour),
info_dbrecord_type(
	variable_name("tpy.time.min"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Minute of tpy file time")
		})),
	"INT", true, update_enum::forever,
	&InfoInterface::info_update_tpy_time_min),
info_dbrecord_type(
	variable_name("tpy.time.sec"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Second of tpy file time")
		})),
	"INT", true, update_enum::forever,
	&InfoInterface::info_update_tpy_time_sec),
info_dbrecord_type(
	variable_name("ads.version"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "ADS library version")
		})),
	"UINT", true, update_enum::once,
	&InfoInterface::info_update_ads_version),
info_dbrecord_type(
	variable_name("ads.revision"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "ADS library revision")
		})),
	"UINT", true, update_enum::once,
	&InfoInterface::info_update_ads_revision),
info_dbrecord_type(
	variable_name("ads.build"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "ADS library build")
		})),
	"UINT", true, update_enum::once,
	&InfoInterface::info_update_ads_build),
info_dbrecord_type(
	variable_name("ads.port"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "ADS/AMS port of PLC")
		})),
	"UINT", true, update_enum::once,
	&InfoInterface::info_update_ads_port),
info_dbrecord_type (
	variable_name("ads.netid.str"),
	process_type_enum::pt_string,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "ADS/AMS address of PLC")
	})),
	"STRING", true, update_enum::once,
	&InfoInterface::info_update_ads_netid_str),
info_dbrecord_type(
	variable_name("ads.netid.b0"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "ADS/AMS address b0")
		})),
	"BYTE", true, update_enum::once,
	&InfoInterface::info_update_ads_netid_b0),
info_dbrecord_type(
	variable_name("ads.netid.b1"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "ADS/AMS address b1")
		})),
	"BYTE", true, update_enum::once,
	&InfoInterface::info_update_ads_netid_b1),
info_dbrecord_type(
	variable_name("ads.netid.b2"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "ADS/AMS address b2")
		})),
	"BYTE", true, update_enum::once,
	&InfoInterface::info_update_ads_netid_b2),
info_dbrecord_type(
	variable_name("ads.netid.b3"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "ADS/AMS address b3")
		})),
	"BYTE", true, update_enum::once,
	&InfoInterface::info_update_ads_netid_b3),
info_dbrecord_type(
	variable_name("ads.netid.b4"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "ADS/AMS address b4")
		})),
	"BYTE", true, update_enum::once,
	&InfoInterface::info_update_ads_netid_b4),
info_dbrecord_type(
	variable_name("ads.netid.b5"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "ADS/AMS address b5")
		})),
	"BYTE", true, update_enum::once,
	&InfoInterface::info_update_ads_netid_b5),
info_dbrecord_type(
	variable_name("svn.local"),
	process_type_enum::pt_bool,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "SVN local modifications"),
		property_el(OPC_PROP_CLOSE, "MODIFIED"),
		property_el(OPC_PROP_OPEN, "COMMITTED")
		})),
	"BOOL", true, update_enum::once,
	&InfoInterface::info_update_svn_local),
info_dbrecord_type (
	variable_name("svn.revision"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "SVN revision if fully committed")
		})),
	"DINT", true, update_enum::once,
	&InfoInterface::info_update_svn_revision),
info_dbrecord_type(
	variable_name("svn.time.str"),
	process_type_enum::pt_string,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "SVN compile time")
		})),
	"STRING", true, update_enum::once,
	&InfoInterface::info_update_svn_time),
info_dbrecord_type (
	variable_name("cb.queue[0].size"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Size of low pri. callback queue")
		})),
	"DINT", true, update_enum::forever,
	&InfoInterface::info_update_callback_queue0_size),
info_dbrecord_type (
	variable_name("cb.queue[0].used"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Used entries in low pri. callback queue")
		})),
	"DINT", true, update_enum::forever,
	&InfoInterface::info_update_callback_queue0_used),
info_dbrecord_type (
	variable_name("cb.queue[0].free"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Free entries low pri. callback queue")
		})),
	"DINT", true, update_enum::forever,
	&InfoInterface::info_update_callback_queue0_free),
info_dbrecord_type (
	variable_name("cb.queue[0].percent"),
	process_type_enum::pt_real,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Use % of low pri. callback queue"),
		property_el(OPC_PROP_HIEU, "1"),
		property_el(OPC_PROP_LOEU, "0"),
		property_el(OPC_PROP_PREC, "1"),
		property_el(OPC_PROP_UNIT, "percent")
		})),
	"LREAL", true, update_enum::forever,
	&InfoInterface::info_update_callback_queue0_percent),
info_dbrecord_type (
	variable_name("cb.queue[1].size"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Size of med pri. callback queue")
		})),
	"DINT", true, update_enum::forever,
	&InfoInterface::info_update_callback_queue1_size),
info_dbrecord_type (
	variable_name("cb.queue[1].used"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Used entries in med pri. callback queue")
		})),
	"DINT", true, update_enum::forever,
	&InfoInterface::info_update_callback_queue1_used),
info_dbrecord_type (
	variable_name("cb.queue[1].free"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Free entries med pri. callback queue")
		})),
	"DINT", true, update_enum::forever,
	&InfoInterface::info_update_callback_queue1_free),
info_dbrecord_type (
	variable_name("cb.queue[1].percent"),
	process_type_enum::pt_real,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Use % of med pri. callback queue"),
		property_el(OPC_PROP_HIEU, "1"),
		property_el(OPC_PROP_LOEU, "0"),
		property_el(OPC_PROP_PREC, "1"),
		property_el(OPC_PROP_UNIT, "percent")
		})),
	"LREAL", true, update_enum::forever,
	&InfoInterface::info_update_callback_queue1_percent),
info_dbrecord_type (
	variable_name("cb.queue[2].size"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Size of hi pri. callback queue")
		})),
	"DINT", true, update_enum::forever,
	&InfoInterface::info_update_callback_queue2_size),
info_dbrecord_type (
	variable_name("cb.queue[2].used"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Used entries in hi pri. callback queue")
		})),
	"DINT", true, update_enum::forever,
	&InfoInterface::info_update_callback_queue2_used),
info_dbrecord_type (
	variable_name("cb.queue[2].free"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Free entries hi pri. callback queue")
		})),
	"DINT", true, update_enum::forever,
	&InfoInterface::info_update_callback_queue2_free),
info_dbrecord_type (
	variable_name("cb.queue[2].percent"),
	process_type_enum::pt_real,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Use % of hi pri. callback queue"),
		property_el(OPC_PROP_HIEU, "1"),
		property_el(OPC_PROP_LOEU, "0"),
		property_el(OPC_PROP_PREC, "1"),
		property_el(OPC_PROP_UNIT, "percent")
		})),
	"LREAL", true, update_enum::forever,
	&InfoInterface::info_update_callback_queue2_percent)
});



/* InfoInterface::InfoInterface
 ************************************************************************/
InfoInterface::InfoInterface (plc::BaseRecord& dval, 
	const std::stringcase& id, const std::stringcase& name, 
	const std::stringcase& tname)
	: Interface(dval), tCatName(name), tCatType(tname), 
	update_freq (update_enum::done), info_update (nullptr)
{
	// Look for variable by name
	auto iter = find_if (dbinfo_list.cbegin(), dbinfo_list.cend(), 
		[n = id](const info_dbrecord_type& info) {
			return (n == get<ParseUtil::variable_name>(info).get_name()); });
	// Nothing found: disable
	if (iter == dbinfo_list.cend()) {
		if (debug) printf("Info record %s not found.\n", id.c_str());
		record.set_process(false);
	}
	// Found it: setup
	else {
		// Set update method & frequency
		update_freq = get<update_enum>(*iter);
		info_update = get<info_update_method>(*iter);
		// check for enum
		if (get<ParseUtil::process_type_enum>(*iter) == pt_enum) {
			tCatType = "ENUM";
		}
		// check for struct
		if (get<ParseUtil::process_type_enum>(*iter) == pt_binary) {
			record.set_process(false);
		}
		if (debug) {
			printf("Info record %s: update %s method %s.\n", id.c_str(),
				(update_freq == update_enum::done) ? "done" : 
					((update_freq == update_enum::once) ? "once" : "forever"),
				info_update ? "set" : "nullptr");
		}
	}
};

/* InfoInterface::InfoInterface
 ************************************************************************/
bool InfoInterface::update() 
{
	if (!info_update) return false;
	if (update_freq == update_enum::done) return true;
	if (update_freq == update_enum::once) update_freq = update_enum::done;
	return (this->*info_update)();
};

/* InfoInterface::printTCatVal
 ************************************************************************/
void InfoInterface::printVal (FILE* fp)
{
	/////////////////////////////////////////////////
	/// This is a function for printing the variable name and value of a record.
	/// Depending on the variable type, the readout from the ADS server is cast
	/// into the proper data type and printed to the output file fp.
	/////////////////////////////////////////////////
	fprintf(fp, "%65s: %15s         ", tCatName.c_str(), tCatType.c_str());

	double				doublePLCVar;
	float				floatPLCVar;
	signed long int		sliPLCVar;
	signed short int	ssiPLCVar;
	signed char			charPLCVar;
	string				chararrPLCVar;

	if (tCatType == "LREAL") {
		if (record.PlcRead (doublePLCVar)) fprintf(fp, "%f", doublePLCVar);
	}
	else if (tCatType == "REAL") {
		if (record.PlcRead(floatPLCVar)) fprintf(fp, "%f", floatPLCVar);
	}
	else if (tCatType == "DWORD" || tCatType == "DINT" || tCatType == "UDINT") {
		if (record.PlcRead(sliPLCVar)) fprintf(fp, "%d", sliPLCVar);
	}
	else if (tCatType == "INT" || tCatType == "WORD" || tCatType == "ENUM" || tCatType == "UINT") {
		if (record.PlcRead(ssiPLCVar)) fprintf(fp, "%d", ssiPLCVar);
	}
	else if (tCatType == "BOOL" || tCatType == "BYTE" || tCatType == "SINT" || tCatType == "USINT") {
		if (record.PlcRead(charPLCVar)) fprintf(fp, "%d", charPLCVar);
	}
	else if (tCatType.substr(0, 6) == "STRING") {
		if (record.PlcRead(chararrPLCVar)) fprintf(fp, "%s", chararrPLCVar.c_str());
	}
	else {
		fprintf(fp, "INVALID!!!");
	}
	fprintf(fp, "\n");
}

/* InfoInterface::info_update_name
 ************************************************************************/
bool InfoInterface::info_update_name ()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	return record.PlcWrite(tc->get_name().c_str(), tc->get_name().size());
}

/* InfoInterface::info_update_alias
 ************************************************************************/
bool InfoInterface::info_update_alias()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	return record.PlcWrite(tc->get_alias().c_str(), tc->get_alias().size());
}

/* InfoInterface::info_update_active
 ************************************************************************/
bool InfoInterface::info_update_active()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	bool active = (tc->get_ads_state() == ADSSTATE_RUN);
	return record.PlcWrite(active);
}

/* InfoInterface::info_update_state
 ************************************************************************/
bool InfoInterface::info_update_state()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	int state = tc->get_ads_state();
	return record.PlcWrite(state);
}

/* InfoInterface::info_update_statestr
 ************************************************************************/
bool InfoInterface::info_update_statestr()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	int state = tc->get_ads_state();
	string str;
	switch (state) {
	case 0: 
		str = "INVALID";
		break;
	case 1:
		str = "IDLE";
		break;
	case 2:
		str = "RESET";
		break;
	case 3:
		str = "INIT";
		break;
	case 4:
		str = "START";
		break;
	case 5:
		str = "RUN";
		break;
	case 6:
		str = "STOP";
		break;
	case 7:
		str = "SAVECFG";
		break;
	case 8:
		str = "LOADCFG";
		break;
	case 9:
		str = "POWERFAILURE";
		break;
	case 10:
		str = "POWERGOOD";
		break;
	case 11:
		str = "ERROR";
		break;
	case 12:
		str = "SHUTDOWN";
		break;
	case 13:
		str = "SUSPEND";
		break;
	case 14:
		str = "RESUME";
		break;
	case 15:
		str = "CONFIG";
		break;
	case 16:
		str = "RECONFIG";
		break;
	case 17:
		str = "STOPPING";
		break;
	default:
		str = "UNKNOWN";
		break;
	}
	return record.PlcWrite(str);
}

/* InfoInterface::info_update_timestamp_str
 ************************************************************************/
bool InfoInterface::info_update_timestamp_str()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	time_t tstamp = tc->get_timestamp_unix();
	tm utc;
	char buf[100];
	if (epicsTime_gmtime(&tstamp, &utc) == 0) {
		strftime(buf, sizeof(buf), "%F %T", &utc);
		return record.PlcWrite(buf, sizeof (buf));
	}
	return false;
}

/* InfoInterface::info_update_timestamp_year
 ************************************************************************/
bool InfoInterface::info_update_timestamp_year()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	time_t tstamp = tc->get_timestamp_unix();
	tm utc;
	if (epicsTime_gmtime(&tstamp, &utc) == 0) {
		return record.PlcWrite(utc.tm_year + 1900);
	}
	return false;
}

/* InfoInterface::info_update_timestamp_month
 ************************************************************************/
bool InfoInterface::info_update_timestamp_month()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	time_t tstamp = tc->get_timestamp_unix();
	tm utc;
	if (epicsTime_gmtime(&tstamp, &utc) == 0) {
		return record.PlcWrite(utc.tm_mon + 1);
	}
	return false;
}

/* InfoInterface::info_update_timestamp_day
 ************************************************************************/
bool InfoInterface::info_update_timestamp_day()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	time_t tstamp = tc->get_timestamp_unix();
	tm utc;
	if (epicsTime_gmtime(&tstamp, &utc) == 0) {
		return record.PlcWrite(utc.tm_mday);
	}
	return false;
}

/* InfoInterface::info_update_timestamp_hour
 ************************************************************************/
bool InfoInterface::info_update_timestamp_hour()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	time_t tstamp = tc->get_timestamp_unix();
	tm utc;
	if (epicsTime_gmtime(&tstamp, &utc) == 0) {
		return record.PlcWrite(utc.tm_hour);
	}
	return false;
}

/* InfoInterface::info_update_timestamp_min
 ************************************************************************/
bool InfoInterface::info_update_timestamp_min()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	time_t tstamp = tc->get_timestamp_unix();
	tm utc;
	if (epicsTime_gmtime(&tstamp, &utc) == 0) {
		return record.PlcWrite(utc.tm_min);
	}
	return false;
}

/* InfoInterface::info_update_timestamp_sec
 ************************************************************************/
bool InfoInterface::info_update_timestamp_sec()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	time_t tstamp = tc->get_timestamp_unix();
	tm utc;
	if (epicsTime_gmtime(&tstamp, &utc) == 0) {
		return record.PlcWrite(utc.tm_sec);
	}
	return false;
}

/* InfoInterface::info_update_rate_read
 ************************************************************************/
bool InfoInterface::info_update_rate_read()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	int rate = tc->get_read_scanner_period() * tc->get_read_scanner_multiple();
	return record.PlcWrite (rate);
}

/* InfoInterface::info_update_rate_write
 ************************************************************************/
bool InfoInterface::info_update_rate_write()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	int rate = tc->get_write_scanner_period();
	return record.PlcWrite(rate);
}

/* InfoInterface::info_update_rate_update
 ************************************************************************/
bool InfoInterface::info_update_rate_update()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	int rate = tc->get_update_scanner_period();
	return record.PlcWrite(rate);
}

/* InfoInterface::info_update_records_num
 ************************************************************************/
bool InfoInterface::info_update_records_num()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	int rate = tc->count();
	return record.PlcWrite(rate);
}

/* InfoInterface::info_update_tpy_filename
 ************************************************************************/
bool InfoInterface::info_update_tpy_filename()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	string tpyfname = tc->get_tpyfilename();
	string::size_type pos;
	while ((tpyfname.size() >= 40) && 
		   ((pos = tpyfname.find('\\')) != string::npos)) {
		tpyfname.erase (0, pos + 1);
	}
	if (tpyfname.size() >= 40) {
		tpyfname.erase (39, string::npos);
	}
	return record.PlcWrite(tpyfname);
}

/* InfoInterface::info_update_tpy_valid
 ************************************************************************/
bool InfoInterface::info_update_tpy_valid()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	bool tpyvalid = tc->is_tpyfile_valid();
	return record.PlcWrite(tpyvalid);
}

/* InfoInterface::info_update_tpy_time_str
 ************************************************************************/
bool InfoInterface::info_update_tpy_time_str()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	time_t tstamp = tc->get_tpyfile_time();
	tm utc;
	char buf[100];
	if (epicsTime_gmtime(&tstamp, &utc) == 0) {
		strftime(buf, sizeof(buf), "%F %T", &utc);
		return record.PlcWrite(buf, sizeof(buf));
	}
	return false;
}

/* InfoInterface::info_update_tpy_time_year
 ************************************************************************/
bool InfoInterface::info_update_tpy_time_year()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	time_t tstamp = tc->get_tpyfile_time();
	tm utc;
	if (epicsTime_gmtime(&tstamp, &utc) == 0) {
		return record.PlcWrite(utc.tm_year + 1900);
	}
	return false;
}

/* InfoInterface::info_update_tpy_time_month
 ************************************************************************/
bool InfoInterface::info_update_tpy_time_month()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	time_t tstamp = tc->get_tpyfile_time();
	tm utc;
	if (epicsTime_gmtime(&tstamp, &utc) == 0) {
		return record.PlcWrite(utc.tm_mon + 1);
	}
	return false;
}

/* InfoInterface::info_update_tpy_time_day
 ************************************************************************/
bool InfoInterface::info_update_tpy_time_day()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	time_t tstamp = tc->get_tpyfile_time();
	tm utc;
	if (epicsTime_gmtime(&tstamp, &utc) == 0) {
		return record.PlcWrite(utc.tm_mday);
	}
	return false;
}

/* InfoInterface::info_update_tpy_time_hour
 ************************************************************************/
bool InfoInterface::info_update_tpy_time_hour()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	time_t tstamp = tc->get_tpyfile_time();
	tm utc;
	if (epicsTime_gmtime(&tstamp, &utc) == 0) {
		return record.PlcWrite(utc.tm_hour);
	}
	return false;
}

/* InfoInterface::info_update_tpy_time_min
 ************************************************************************/
bool InfoInterface::info_update_tpy_time_min()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	time_t tstamp = tc->get_tpyfile_time();
	tm utc;
	if (epicsTime_gmtime(&tstamp, &utc) == 0) {
		return record.PlcWrite(utc.tm_min);
	}
	return false;
}

/* InfoInterface::info_update_tpy_time_sec
 ************************************************************************/
bool InfoInterface::info_update_tpy_time_sec()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	time_t tstamp = tc->get_tpyfile_time();
	tm utc;
	if (epicsTime_gmtime(&tstamp, &utc) == 0) {
		return record.PlcWrite(utc.tm_sec);
	}
	return false;
}

/* InfoInterface::info_update_ads_version
 ************************************************************************/
bool InfoInterface::info_update_ads_version()
{
	return record.PlcWrite (
		TcComms::AmsRouterNotification::get_instance().get_ads_version());
}

/* InfoInterface::info_update_ads_revision
 ************************************************************************/
bool InfoInterface::info_update_ads_revision()
{
	return record.PlcWrite(
		TcComms::AmsRouterNotification::get_instance().get_ads_revision());
}

/* InfoInterface::info_update_ads_build
 ************************************************************************/
bool InfoInterface::info_update_ads_build()
{
	return record.PlcWrite(
		TcComms::AmsRouterNotification::get_instance().get_ads_build());
}

/* InfoInterface::info_update_ads_port
 ************************************************************************/
bool InfoInterface::info_update_ads_port()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	AmsAddr addr = tc->get_addr();
	return record.PlcWrite(addr.port);
}

/* InfoInterface::info_update_ads_netid_str
 ************************************************************************/
bool InfoInterface::info_update_ads_netid_str()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	AmsAddr addr = tc->get_addr();
	char buf[100];
	snprintf(buf, sizeof(buf), "%u.%u.%u.%u.%u.%u", 
		addr.netId.b[0], addr.netId.b[1], addr.netId.b[2], 
		addr.netId.b[3], addr.netId.b[4], addr.netId.b[5]);
	string netid (buf);
	return record.PlcWrite(netid);
}

/* InfoInterface::info_update_ads_netid_b0
 ************************************************************************/
bool InfoInterface::info_update_ads_netid_b0()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	AmsAddr addr = tc->get_addr();
	return record.PlcWrite(addr.netId.b[0]);
}

/* InfoInterface::info_update_ads_netid_b1
 ************************************************************************/
bool InfoInterface::info_update_ads_netid_b1()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	AmsAddr addr = tc->get_addr();
	return record.PlcWrite(addr.netId.b[1]);
}

/* InfoInterface::info_update_ads_netid_b2
 ************************************************************************/
bool InfoInterface::info_update_ads_netid_b2()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	AmsAddr addr = tc->get_addr();
	return record.PlcWrite(addr.netId.b[2]);
}

/* InfoInterface::info_update_ads_netid_b3
 ************************************************************************/
bool InfoInterface::info_update_ads_netid_b3()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	AmsAddr addr = tc->get_addr();
	return record.PlcWrite(addr.netId.b[3]);
}

/* InfoInterface::info_update_ads_netid_b4
 ************************************************************************/
bool InfoInterface::info_update_ads_netid_b4()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	AmsAddr addr = tc->get_addr();
	return record.PlcWrite(addr.netId.b[4]);
}

/* InfoInterface::info_update_ads_netid_b5
 ************************************************************************/
bool InfoInterface::info_update_ads_netid_b5()
{
	const TcComms::TcPLC* tc = dynamic_cast<const TcComms::TcPLC*>(get_parent());
	if (!tc) return false;
	AmsAddr addr = tc->get_addr();
	return record.PlcWrite(addr.netId.b[5]);
}

/* InfoInterface::info_update_svn_local
 ************************************************************************/
bool InfoInterface::info_update_svn_local()
{
	return record.PlcWrite(svn_local_modifications);
}

/* InfoInterface::info_update_svn_revision
 ************************************************************************/
bool InfoInterface::info_update_svn_revision()
{
	return record.PlcWrite(svn_revision_committed);
}

/* InfoInterface::info_update_svn_time
 ************************************************************************/
bool InfoInterface::info_update_svn_time()
{
	std::string	svn_time (svn_time_now);
	string::size_type pos;
	while ((pos = svn_time.find('/')) != string::npos) {
		svn_time[pos] = '-';
	}
	return record.PlcWrite (svn_time);
}

/* InfoInterface::info_update_callback_queue0_size
 ************************************************************************/
bool InfoInterface::info_update_callback_queue0_size()
{
	return record.PlcWrite (get_callback_queue_size(0));
}

/* InfoInterface::info_update_callback_queue0_used
 ************************************************************************/
bool InfoInterface::info_update_callback_queue0_used()
{
	return record.PlcWrite(get_callback_queue_used(0));
}

/* InfoInterface::info_update_callback_queue0_free
 ************************************************************************/
bool InfoInterface::info_update_callback_queue0_free()
{
	return record.PlcWrite(get_callback_queue_free(0));
}

/* InfoInterface::info_update_callback_queue0_percent
 ************************************************************************/
bool InfoInterface::info_update_callback_queue0_percent()
{
	double sz = (double)get_callback_queue_size(0);
	double usd = (double)get_callback_queue_used(0);
	if ((sz > 1.0) && (usd > 1.0)) {
		return record.PlcWrite(usd/sz);
	}
	else {
		return 0.0;
	}
}

/* InfoInterface::info_update_callback_queue1_size
 ************************************************************************/
bool InfoInterface::info_update_callback_queue1_size()
{
	return record.PlcWrite(get_callback_queue_size(1));
}

/* InfoInterface::info_update_callback_queue1_used
 ************************************************************************/
bool InfoInterface::info_update_callback_queue1_used()
{
	return record.PlcWrite(get_callback_queue_used(1));
}

/* InfoInterface::info_update_callback_queue1_free
 ************************************************************************/
bool InfoInterface::info_update_callback_queue1_free()
{
	return record.PlcWrite(get_callback_queue_free(1));
}

/* InfoInterface::info_update_callback_queue1_percent
 ************************************************************************/
bool InfoInterface::info_update_callback_queue1_percent()
{
	double sz = (double)get_callback_queue_size(1);
	double usd = (double)get_callback_queue_used(1);
	if ((sz > 1.0) && (usd > 1.0)) {
		return record.PlcWrite(usd / sz);
	}
	else {
		return 0.0;
	}
}

/* InfoInterface::info_update_callback_queue2_size
 ************************************************************************/
bool InfoInterface::info_update_callback_queue2_size()
{
	return record.PlcWrite(get_callback_queue_size(2));
}

/* InfoInterface::info_update_callback_queue2_used
 ************************************************************************/
bool InfoInterface::info_update_callback_queue2_used()
{
	return record.PlcWrite(get_callback_queue_used(2));
}

/* InfoInterface::info_update_callback_queue2_free
 ************************************************************************/
bool InfoInterface::info_update_callback_queue2_free()
{
	return record.PlcWrite(get_callback_queue_free(2));
}

/* InfoInterface::info_update_callback_queue2_percent
 ************************************************************************/
bool InfoInterface::info_update_callback_queue2_percent()
{
	double sz = (double)get_callback_queue_size(2);
	double usd = (double)get_callback_queue_used(2);
	if ((sz > 1.0) && (usd > 1.0)) {
		return record.PlcWrite(usd / sz);
	}
	else {
		return 0.0;
	}
}


/* process_arg::get
 ************************************************************************/
std::stringcase process_arg_info::get_full() const
{
	std::stringcase servername (tcplc_addr);
	servername += "info/";
	servername += get_name();
	return servername;
}

}
