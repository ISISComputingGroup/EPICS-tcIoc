#define _CRT_SECURE_NO_WARNINGS
#include "drvInfo.h"
#include "ParseTpy.h"
#include "TpyToEpics.h"
#include "infoPlc.h"
#include "gdd.h"
#include "dbStaticLib.h"
#include "dbAccess.h"
#include "dbDefs.h"
#include "dbFldTypes.h"
#include "link.h"
#include "iocsh.h"
#include "epicsRelease.h"
#include "errlog.h"
#include "iocInit.h"
#include "waveformRecord.h"
#include "initHooks.h"
#undef _CRT_SECURE_NO_WARNINGS

/** @file drvInfo.cpp
	This contains functions for driver support for EPICS. These routines 
	are used during the IOC initialization, and allow the IOC to 
	initialize the TwinCAT interface during EPICS initialization.
 ************************************************************************/

using namespace std;
using namespace ParseUtil;
using namespace ParseTpy;
using namespace EpicsTpy;
using namespace InfoPlc;


namespace DevInfo {

static const iocshArg infoLoadRecordsArg0			= {"'db' Filename (output)", iocshArgString};
static const iocshArg infoLoadRecordsArg1	        = {"Conversion rules", iocshArgString};
static const iocshArg infoSetScanRateArg0			= {"info scan rate in ms", iocshArgString};
static const iocshArg infoListArg0			        = {"'list' Filename", iocshArgString};
static const iocshArg infoListArg1		            = {"Conversion rules", iocshArgString};
static const iocshArg infoAliasArg0					= {"Alias name for PLC", iocshArgString};
static const iocshArg infoPrefixArg0				= {"Tag prefix for info names", iocshArgString};
static const iocshArg infoPrintValsArg0				= {"emptyarg", iocshArgString};

static const iocshArg* const  infoLoadRecordsArg[2]	= {&infoLoadRecordsArg0, &infoLoadRecordsArg1};
static const iocshArg* const  infoSetScanRateArg[1]	= {&infoSetScanRateArg0};
static const iocshArg* const  infoListArg[2]	    = {&infoListArg0, &infoListArg1};
static const iocshArg* const  infoAliasArg[1]		= {&infoAliasArg0};
static const iocshArg* const  infoPrefixArg[1]		= {&infoPrefixArg0};
static const iocshArg* const  infoPrintValsArg[1]	= {&infoPrintValsArg0};

static const iocshFuncDef infoLoadRecordsFuncDef	= {"infoLoadRecords", 2, infoLoadRecordsArg};
static const iocshFuncDef infoSetScanRateFuncDef	= {"infoSetScanRate", 1, infoSetScanRateArg};
static const iocshFuncDef infoListFuncDef		    = {"infoList", 2, infoListArg};
static const iocshFuncDef infoAliasFuncDef		    = {"infoAlias", 1, infoAliasArg};
static const iocshFuncDef infoPrefixFuncDef			= {"infoPrefix", 1, infoPrefixArg};
static const iocshFuncDef infoPrintValsFuncDef		= {"infoPrintVals", 1, infoPrintValsArg};

/** Class for storing a flanem and a rule
	@brief filename/rule pair
 ************************************************************************/
typedef std::pair<std::stringcase, std::stringcase> filename_rule_pair;

static int scanrate = default_scanrate;
static std::stringcase info_prefix;
static std::stringcase info_alias;
static std::vector<filename_rule_pair> info_lists;

/** Class for generating an EPICS database and info record 
    @brief EPICS/Info db processing
 ************************************************************************/
	class epics_info_db_processing : public epics_db_processing {
	public:
		/// Default constructor
		explicit epics_info_db_processing (InfoPLC& p)
			: plc (&p), invnum (0) { 
			device_support = device_support_tc_name; }

		/// Process a variable
		/// @param arg Process argument describign the variable and type
		/// @return True if successful
		bool operator() (const ParseUtil::process_arg& arg);

		/// Get number of EPICS records without info records
		int get_invalid_records() const { return invnum; }

	protected:
		/// Pointer to PLC class
		InfoPLC*	plc;
		/// Number of EPICS records without info records
		int			invnum;
	};

/* Process a channel
   epics_info_db_processing::operator()
 ************************************************************************/
bool epics_info_db_processing::operator() (const ParseUtil::process_arg& arg)
{
	// Generate EPICS database record
	if (!EpicsTpy::epics_db_processing::operator()(arg)) {
		return false;
	}

	/// Determine data type of this record
	plc::data_type_enum rt = plc::data_type_enum::dtInvalid;

	if (arg.get_type_name() == "BOOL") 
		rt = plc::data_type_enum::dtBool;
	else if (arg.get_type_name() == "SINT") 
		rt = plc::data_type_enum::dtInt8;
	else if (arg.get_type_name() == "USINT" || arg.get_type_name() == "BYTE") 
		rt = plc::data_type_enum::dtUInt8;
	else if (arg.get_type_name() == "INT"|| arg.get_process_type() == pt_enum) 
		rt = plc::data_type_enum::dtInt16;
	else if (arg.get_type_name() == "UINT" || arg.get_type_name() =="WORD") 
		rt = plc::data_type_enum::dtUInt16;
	else if (arg.get_type_name() == "DINT") 
		rt = plc::data_type_enum::dtInt32;
	else if (arg.get_type_name() == "UDINT" || arg.get_type_name() == "DWORD") 
		rt = plc::data_type_enum::dtUInt32;
	else if (arg.get_type_name() == "REAL") 
		rt = plc::data_type_enum::dtFloat;
	else if (arg.get_type_name() == "LREAL") 
		rt = plc::data_type_enum::dtDouble;
	else if (arg.get_type_name().substr(0,6) == "STRING") 
		rt = plc::data_type_enum::dtString;

	/// Make new record object
	plc::BaseRecord* rec = new (std::nothrow) plc::BaseRecord (arg.get_full(), rt);
	if (!rec) {
		// this means the allocation failed!
		++invnum;
		return false;
	}
	plc::BaseRecordPtr pRecord (rec);
	/// Make info interface
	InfoInterface* info = new (std::nothrow) InfoInterface (*pRecord, 
								arg.get_name());
	if (info == 0) {
		// this means the allocation failed!
		++invnum;
		return false;
	}

	/// Tell record about info interface
	pRecord->set_plcInterface(info);

	if (!plc->add(pRecord)) {
		// this means the EPICS record has nothing to connect to!
		++invnum;
		return false;
	}

	return true;
}


/** @defgroup iocshfunc Functions called by the EPICS base
 ************************************************************************/
/** @{ */

/** Function for loading the info record, and using it to generate 
	internal record entries as well as the EPICs .db file
	@brief Info load records
 ************************************************************************/
void infoLoadRecords (const iocshArgBuf *args) 
{
	// save and reset alias name and listings
	std::stringcase alias = info_alias;
	std::stringcase prefix = info_prefix;
	std::vector<filename_rule_pair> listings = info_lists;
	info_alias = "";
	info_prefix = "";
	info_lists.clear();

	// Check if Ioc is running
    if (plc::System::get().is_ioc_running()) {
        printf ("IOC is already initialized\n");
        return;
    }

	// check output file
	if (!args || !args[0].sval || (strlen (args[0].sval) == 0)) {
        printf("Specify a database file\n");
		return;
	}
	// check option arguments
	optarg options;
	if (args[1].sval) {
		options.parse (args[1].sval);
	}

	// Timer for just tpy file parsing
	clock_t infobegin;
	clock_t infoend;

	infobegin = clock();
	
	// generate the db filename
	stringcase outfilename (args[0].sval);
	// get plc
	plc::BasePLCPtr infoplc = plc::System::get().find ("info://0.0.0.0.0.0:1/");
	if (!infoplc) {
		return;
	}
	InfoPLC* iplc = dynamic_cast<InfoPLC*> (infoplc.get());
	if (!iplc) {
		return;
	}
	// set plc parameters
	iplc->set_read_scanner_period (scanrate);
	iplc->set_alias (alias);
	iplc->set_prefix (info_prefix);
	
	// Set up output db generator
	epics_info_db_processing dbproc (*iplc);
	// default conversion rules
	(epics_conversion&)(dbproc) = 
		epics_conversion (ligo_std, upper_case, false, true);
	// option processing
	dbproc.getopt (options.argc(), options.argv(), options.argp());
	// force device support for tc
	dbproc.set_device_support (device_support_tc_name);
	// force single file
	split_io_support iosupp (outfilename, false, 0);
	if (!iosupp) {
		printf ("Failed to open output %s.\n", outfilename.c_str());
		return;
	}
	(split_io_support&)(dbproc) = iosupp;

	// generate db file and info records
	int num = iplc->process_info (dbproc, info_prefix);
	if (dbproc.get_invalid_records() == 0) {
		printf ("Loaded %i records from %s.\n", num, args[0].sval);
	}
	else {
		printf ("Loaded %i valid and %i invaid records from %s.\n", 
			num, dbproc.get_invalid_records(), args[0].sval);
	}
	// make sure all file contents is written to file
	dbproc.flush();
	
	// end timer
	infoend = clock();
	printf("Info parsing took %f seconds.\n",((float)(infoend - infobegin)/CLOCKS_PER_SEC));

	if (!infoplc->start ()) {
		printf ("Failed to start\n");
		return;
	}

	// load epics database
	stringcase::size_type pos = outfilename.rfind ('\\');
	stringcase path;
	stringcase fname;
	if (pos == stringcase::npos) {
		fname = outfilename;
	}
	else {
		fname = outfilename.substr (pos + 1);
		path  = outfilename.substr (0, pos);
	}

	{
		printf ("Loading record database %s.\n", outfilename.c_str());
		if (dbLoadRecords (outfilename.c_str(), 0)) {
			printf ("\nUnable to laod record database for %s.\n", outfilename.c_str());
			return;
		}
		printf ("Loaded record database %s.\n", outfilename.c_str());
	}
	// success!

    return;
}

/** Set scan rate of the read scanner
	@brief Info set scan rate
 ************************************************************************/
void infoSetScanRate (const iocshArgBuf *args) 
{
	// Check if Ioc is running
    if (plc::System::get().is_ioc_running()) {
        printf ("IOC is already initialized\n");
        return;
    }

	// Check arguments
	if (!args) {
        printf("Specify a scan rate\n");
		return;
	}
	const char* p1 = args[0].sval;
	const char* p2 = args[1].sval;
	if (!p1 || !p2) {
        printf("Specify a scan rate for info and epics\n");
		return;
	}
	// Convert to number
	char* pp;
    scanrate = strtol (p1, &pp, 10);
	if (*pp) {
        printf("Scan rate must be an integer %s\n", p1);
		return;
	}
	if (scanrate < minimum_scanrate) {
		scanrate = minimum_scanrate;
        printf("Scan rate set to minimum %i ms\n", scanrate);
	}
	if (scanrate > maximum_scanrate) {
		scanrate = maximum_scanrate;
        printf("Scan rate set to maximum %i ms\n", scanrate);
	}
	printf ("Scan rate is %i ms.\n", scanrate);
    return;
}

/** List function to generate separate listings
    @brief channel lists
 ************************************************************************/
void infoList (const iocshArgBuf *args)
{
	// Check if Ioc is running
    if (plc::System::get().is_ioc_running()) {
        printf ("IOC is already initialized\n");
        return;
    }
	// Check arguments
	if (!args) {
        printf("Specify a list filename\n");
		return;
	}
	const char* p1 = args[0].sval;
	const char* p2 = args[1].sval;
	if (!p1 || !p2) {
        printf("Specify a list filename and conversion rules for tc and epics\n");
		return;
	}
	info_lists.push_back (filename_rule_pair (p1, p2));
}

/** Define a nick name or alias
    @brief alias
 ************************************************************************/
void infoAlias (const iocshArgBuf *args)
{
	// Check if Ioc is running
    if (plc::System::get().is_ioc_running()) {
        printf ("IOC is already initialized\n");
        return;
    }
	// Check arguments
	if (!args) {
        printf("Specify an alias\n");
		return;
	}
	const char* p1 = args[0].sval;
	if (!p1) {
        printf("Specify an alias name for a tc PLC\n");
		return;
	}
	info_alias = p1;
}

/** Define the tag prefix
    @brief tag prefix
 ************************************************************************/
void infoPrefix (const iocshArgBuf *args)
{
	// Check if Ioc is running
    if (plc::System::get().is_ioc_running()) {
        printf ("IOC is already initialized\n");
        return;
    }
	// Check arguments
	if (!args) {
        printf("Specify an alias\n");
		return;
	}
	const char* p1 = args[0].sval;
	if (!p1) {
        printf("Specify an alias name for a tc PLC\n");
		return;
	}
	info_prefix = p1;
}

/** Debugging function that prints the values for all records on the PLCs
	@brief Info print vals
 ************************************************************************/
void infoPrintVals (const iocshArgBuf *args)
{
	plc::System::get().printVals();
	return;
}

/** Register functions to EPICS IOC shell
	@brief Register to iocsh
 ************************************************************************/
InfoRegisterToIocShell::InfoRegisterToIocShell() 
{
    iocshRegister(&infoLoadRecordsFuncDef, infoLoadRecords);
    iocshRegister(&infoSetScanRateFuncDef, infoSetScanRate);
    iocshRegister(&infoListFuncDef, infoList);
    iocshRegister(&infoAliasFuncDef, infoAlias);
    iocshRegister(&infoPrefixFuncDef, infoPrefix);
	iocshRegister(&infoPrintValsFuncDef, infoPrintVals);
}

/// create a static object to make shure that opcRegisterToIocShell is called on beginning of code
InfoRegisterToIocShell InfoRegisterToIocShell::gInfoRegisterToIocShell;

/** @} */

}

