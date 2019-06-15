#include "infoPlc.h"

using namespace std;
using namespace ParseUtil;
using namespace plc;

/** @file InfoPlc.cpp
	Defines methods for the info PLC.
 ************************************************************************/

namespace InfoPlc {


/// List of of db info tuples
info_dbrecord_list InfoInterface::dbinfo_list({
info_dbrecord_type(
	variable_name("name"),
	process_type_enum::pt_string,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "PCL name")
	})),
	"STRING", true),
info_dbrecord_type(
	variable_name("alias"),
	process_type_enum::pt_string,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Alias name")
		})),
	"STRING", true),
info_dbrecord_type(
	variable_name("active"),
	process_type_enum::pt_bool,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "Running state of PLC"),
		property_el(OPC_PROP_CLOSE, "ONLINE"),
		property_el(OPC_PROP_OPEN, "OFFLINE")
		})),
	"BOOL", true),
info_dbrecord_type (
	variable_name("state"),
	process_type_enum::pt_int,
	opc_list(publish, property_map({
		property_el(OPC_PROP_RIGHTS, "1"),
		property_el(OPC_PROP_DESC, "AMS state of PLC")
		})),
	"DINT", true)
	});

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
	field(DESC,"Alias name")
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



/* InfoInterface::InfoInterface
 ************************************************************************/
InfoInterface::InfoInterface (plc::BaseRecord& dval,
	const std::stringcase& name, const std::stringcase& tname, 
	bool isStruct, bool isEnum)
	: Interface(dval), tCatName(name), tCatType(tname) 
{
	if (isEnum)	tCatType = "ENUM";
	if (isStruct) record->set_process(false);
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
	fprintf(fp, "%15s: %15s         ", tCatName.c_str(), tCatType.c_str());

	/*double				doublePLCVar;
	float				floatPLCVar;
	signed long int		sliPLCVar;
	signed short int	ssiPLCVar;
	signed char			charPLCVar;
	char				chararrPLCVar[100];
	void*				pTCatVal = nullptr;

	if (tCatType == "LREAL")
	{
		doublePLCVar = *(double*)pTCatVal;
		fprintf(fp, "%f", doublePLCVar);
	}
	else if (tCatType == "REAL")
	{
		floatPLCVar = *(float*)pTCatVal;
		fprintf(fp, "%f", floatPLCVar);
	}
	else if (tCatType == "DWORD" || tCatType == "DINT" || tCatType == "UDINT")
	{
		sliPLCVar = *(signed long int*)pTCatVal;
		fprintf(fp, "%d", sliPLCVar);
	}
	else if (tCatType == "INT" || tCatType == "WORD" || tCatType == "ENUM" || tCatType == "UINT")
	{
		ssiPLCVar = *(signed short int*)pTCatVal;
		fprintf(fp, "%d", ssiPLCVar);
	}
	else if (tCatType == "BOOL" || tCatType == "BYTE" || tCatType == "SINT" || tCatType == "USINT")
	{
		charPLCVar = *(signed char*)pTCatVal;
		fprintf(fp, "%d", charPLCVar);
	}
	else if (tCatType.substr(0, 6) == "STRING")
	{
		//strncpy(chararrPLCVar, (char*)pTCatVal, tCatSymbol.length);
		fprintf(fp, "%s", chararrPLCVar);
	}
	else
	{
		fprintf(fp, "INVALID!!!");
	}*/

	fprintf(fp, "\n");
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