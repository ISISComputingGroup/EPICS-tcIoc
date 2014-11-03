#define _CRT_SECURE_NO_WARNINGS
#include "drvTc.h"
#include "ParseTpy.h"
#include "TpyToEpics.h"
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
#include "tcComms.h"
#include "epicsExit.h"
#undef _CRT_SECURE_NO_WARNINGS

/** @file drvTc.cpp
	This contains functions for driver support for EPICS. These routines 
	are used during the IOC initialization, and allow the IOC to 
	initialize the TwinCAT interface during EPICS initialization.
 ************************************************************************/

bool dbg = 0;

using namespace std;
using namespace ParseUtil;
using namespace ParseTpy;
using namespace EpicsTpy;


namespace DevTc {

static const iocshArg tcLoadRecordsArg0	            = {"'tpy' Filename", iocshArgString};
static const iocshArg tcLoadRecordsArg1	            = {"Conversion rules", iocshArgString};
static const iocshArg tcSetScanRateArg0	            = {"TC scan rate in ms", iocshArgString};
static const iocshArg tcSetScanRateArg1	            = {"EPICS scan rate in multiples of the TC scan rate", iocshArgString};
static const iocshArg tcListArg0			        = {"'list' Filename", iocshArgString};
static const iocshArg tcListArg1		            = {"Conversion rules", iocshArgString};
static const iocshArg tcAliasArg0					= {"Alias name for PLC", iocshArgString};
static const iocshArg tcPrintValsArg0	            = {"emptyarg", iocshArgString};

static const iocshArg* const  tcLoadRecordsArg[2]   = {&tcLoadRecordsArg0, &tcLoadRecordsArg1};
static const iocshArg* const  tcSetScanRateArg[2]   = {&tcSetScanRateArg0, &tcSetScanRateArg1};
static const iocshArg* const  tcListArg[2]		    = {&tcListArg0, &tcListArg1};
static const iocshArg* const  tcAliasArg[1]			= {&tcAliasArg0};
static const iocshArg* const  tcPrintValsArg[1]		= {&tcPrintValsArg0};

iocshFuncDef tcLoadRecordsFuncDef                   = {"tcLoadRecords", 2, tcLoadRecordsArg};
iocshFuncDef tcSetScanRateFuncDef		            = {"tcSetScanRate", 2, tcSetScanRateArg};
iocshFuncDef tcListFuncDef				            = {"tcGenerateList", 2, tcListArg};
iocshFuncDef tcAliasFuncDef				            = {"tcSetAlias", 1, tcAliasArg};
iocshFuncDef tcPrintValsFuncDef				        = {"tcPrintVals", 1, tcPrintValsArg};

typedef std::tuple<std::stringcase, std::stringcase, 
				   epics_list_processing*, bool> filename_rule_list_tuple;
typedef std::vector<filename_rule_list_tuple> tc_listing_def;

static int scanrate = TcComms::default_scanrate;
static int multiple = TcComms::default_multiple;
static std::stringcase tc_alias;
static tc_listing_def tc_lists;


/* Class for generating an EPICS database and tc record 
	@brief EPICS/TCat db processing
 ************************************************************************/
class epics_tc_db_processing : public EpicsTpy::epics_db_processing {
public:
	/// Default constructor
	explicit epics_tc_db_processing (TcComms::TcPLC& p,
		tc_listing_def* l = nullptr)
		: plc (&p), invnum (0), lists (l) { init(); }
	~epics_tc_db_processing() { done(); }

	/// Process a variable
	/// @param arg Process argument describign the variable and type
	/// @return True if successful
	bool operator() (const ParseUtil::process_arg& arg);

	/// Get number of EPICS records without tc records
	int get_invalid_records() const { return invnum; }

protected:
	/// Disable copy contructor
	epics_tc_db_processing (const epics_tc_db_processing&);
	/// Disable assignment operator
	epics_tc_db_processing& operator= (const epics_tc_db_processing&);

	/// Init lists
	void init();
	/// Cleanup lists
	void done();
	/// Process all listings
	/// @param arg Process argument describign the variable and type
	/// @return True if successful
	bool process (const ParseUtil::process_arg& arg);
	/// Process a listing
	/// @param listdef filename/rule pair defining a listing
	/// @param arg Process argument describign the variable and type
	/// @return True if successful
	bool process (filename_rule_list_tuple& listdef, 
		const ParseUtil::process_arg& arg);

	/// Pointer to PLC class
	TcComms::TcPLC*		plc;
	/// Pointer to a set of listings
	tc_listing_def*		lists;
	/// Number of EPICS records without tc records
	int					invnum;
};

/* epics_tc_db_processing::init
 ************************************************************************/
void epics_tc_db_processing::init()
{
	device_support = device_support_tc_name;
	if (!lists) return;
	for (filename_rule_list_tuple& list : *lists) {
		optarg options (get<1>(list));
		epics_list_processing* lproc = new (std::nothrow) epics_list_processing;
		bool no_string = false;
		if (lproc) {
			// look for no string argument
			for (int i = 1; i < options.argc(); ++i) {
				if (options.argp() && options.argp()[i]) continue;
				if (!options.argv()[i]) {
					if (options.argp()) options.argp()[i] = true;
					continue;
				}
				std::stringcase arg (options.argv()[i]);
				// export only opc variables (default)
				if (arg == "-ns" || arg == "/ns" ) {
					no_string = true;
					options.argp()[i] = true;
				}
			}
			// option processing
			lproc->getopt (options.argc(), options.argv(), options.argp());
			// force single file
			split_io_support iosupp (std::get<0>(list), false, 0);
			if (!iosupp) {
				printf ("Failed to open output %s.\n", std::get<0>(list).c_str());
				delete lproc;
				lproc = nullptr;
			}
			else {
				(split_io_support&)(*lproc) = iosupp;
			}
		}
		if (std::get<2>(list)) delete std::get<2>(list);
		std::get<2>(list) = lproc;
		std::get<3>(list) = no_string;
	}
}

/* epics_tc_db_processing::done
 ************************************************************************/
void epics_tc_db_processing::done()
{
	if (!lists) return;
	for (filename_rule_list_tuple& list : *lists) {
		if (std::get<2>(list)) {
			delete std::get<2>(list);
			std::get<2>(list) = nullptr;
		}
	}
}

/* Process a channel
   epics_tc_db_processing::operator()
 ************************************************************************/
bool epics_tc_db_processing::process (const ParseUtil::process_arg& arg)
{
	if (!lists) return true;
	bool succ = true;
	for (filename_rule_list_tuple& i : *lists) {
		if (!process (i, arg)) succ = false;
	}
	return succ;
}

/* Process a channel
   epics_tc_db_processing::operator()
 ************************************************************************/
bool epics_tc_db_processing::process (filename_rule_list_tuple& listdef,
									  const ParseUtil::process_arg& arg)
{
	epics_list_processing* lptr = std::get<2>(listdef);
	if (!lptr) return false;
	if (std::get<3>(listdef) && arg.get_process_type() == pt_string) return false;
	return (*lptr) (arg);
}

/* Process a channel
   epics_tc_db_processing::operator()
 ************************************************************************/
bool epics_tc_db_processing::operator() (const ParseUtil::process_arg& arg)
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
	plc::BaseRecordPtr pRecord = plc::BaseRecordPtr(new plc::BaseRecord(arg.get_full(),rt));
	/// Make TCat interface
	TcComms::TCatInterface* tcat = new (std::nothrow) TcComms::TCatInterface (*pRecord, 
								arg.get_name(), 
								arg.get_igroup(), 
								arg.get_ioffset(), 
								arg.get_bytesize(), 
								arg.get_type_name(), 
								arg.get_process_type() == pt_binary, 
								arg.get_process_type() == pt_enum);
	if (tcat == 0) {
		// this means the allocation failed!
		++invnum;
		return false;
	}

	/// Tell record about TCat interface
	pRecord->set_plcInterface(tcat);

	if (!plc->add(pRecord)) {
		// this means the EPICS record has nothing to connect to!
		++invnum;
		return false;
	}

	process (arg);
	return true;
}


/** @defgroup iocshfunc Functions called by the EPICS base
 ************************************************************************/
/** @{ */

/* Function for loading a TCat tpy file, and using it to generate 
	internal record entries as well as the EPICs .db file
	@brief TCat load records
 ************************************************************************/
void tcLoadRecords (const iocshArgBuf *args) 
{
	// save and reset alias name and listings
	std::stringcase alias = tc_alias;
	tc_listing_def listings = tc_lists;
	tc_alias = "";
	tc_lists.clear();

	// Check if Ioc is running
	if (plc::System::get().is_ioc_running()) {
        printf ("IOC is already initialized\n");
        return;
    }

	// open input file
	if (!args || !args[0].sval || (strlen (args[0].sval) == 0)) {
        printf("Specify a tpy filename\n");
		return;
	}
	FILE* inpf = 0;
	inpf = fopen (args[0].sval, "r");
	if (!inpf) {
		printf ("Failed to open input %s.\n", args[0].sval);
		return;
	}
	// check option arguments
	optarg options;
	if (args[1].sval) {
		options.parse (args[1].sval);
	}

	// Timer for just tpy file parsing
	clock_t tpybegin;
	clock_t tpyend;

	tpybegin = clock();

	// parse tpy file
	ParseTpy::tpy_file tpyfile;
	tpyfile.getopt (options.argc(), options.argv(), options.argp());
	if (!tpyfile.parse (inpf)) {
		printf ("Unable to parse %s.\n", args[0].sval);
		fclose (inpf);
		return;
	}
	fclose (inpf);

	// generate the db filename
	stringcase outfilename (args[0].sval);
	stringcase::size_type pos = outfilename.rfind (".tpy");
	if (pos == outfilename.length() - 4) {
		outfilename.erase (pos);
	}
	outfilename.append (".db");

	// get ADS parameters
	stringcase netid = tpyfile.get_project_info().get_netid();
	int port = tpyfile.get_project_info().get_port();

	// get plc
	TcComms::TcPLC* tcplc = new (std::nothrow) TcComms::TcPLC(args[0].sval);
	if (!tcplc) {
		printf ("Failed to allocate PLC %s.\n", outfilename.c_str());
		return;
	}
	// set plc parameters
	tcplc->set_addr(netid, port);
	tcplc->set_read_scanner_period (scanrate);
	tcplc->set_write_scanner_period (scanrate);
	tcplc->set_update_scanner_period (scanrate);
	tcplc->set_read_scanner_multiple (multiple);
	tcplc->set_alias (alias);
	
	// Set up output db generator
	epics_tc_db_processing dbproc (*tcplc, &listings);
//+ without listings	epics_tc_db_processing dbproc (*tcplc);
	// option processing
	dbproc.getopt (options.argc(), options.argv(), options.argp());
	// force single file
	split_io_support iosupp (outfilename, false, 0);
	if (!iosupp) {
		printf ("Failed to open output %s.\n", outfilename.c_str());
		return;
	}
	(split_io_support&)(dbproc) = iosupp;

	// generate db file and tc records
	if (dbg) tpyfile.set_export_all (TRUE);
	int num = tpyfile.process_symbols (dbproc);
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
	tpyend = clock();
	printf("Tpy parsing took %f seconds.\n",((float)(tpyend - tpybegin)/CLOCKS_PER_SEC));

	//Start scanner
	if (!tcplc->optimizeRequests()) {
		printf ("Failed to optimize request groups\n");
		return;
	}

	if (!tcplc->start ()) {
		printf ("Failed to start\n");
		return;
	}

	plc::System::get().add(plc::BasePLCPtr(tcplc)); // adopted by TSystem

	// load epics database
	pos = outfilename.rfind ('\\');
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
			printf ("Unable to laod record database for %s.\n", outfilename.c_str());
			return;
		}
		printf ("Loaded record database %s.\n", outfilename.c_str());
	}
	// success!

	return;
}

/* Set scan rate of the read scanner
	@brief TCat set scan rate
 ************************************************************************/
void tcSetScanRate (const iocshArgBuf *args) 
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
        printf("Specify a scan rate for tc and epics\n");
		return;
	}
	// Convert to number
	char* pp;
    scanrate = strtol (p1, &pp, 10);
	if (*pp) {
        printf("Scan rate must be an integer %s\n", p1);
		return;
	}
	if (scanrate < TcComms::minimum_scanrate) {
		scanrate = TcComms::minimum_scanrate;
        printf("Scan rate set to minimum %i ms\n", scanrate);
	}
	if (scanrate > TcComms::maximum_scanrate) {
		scanrate = TcComms::maximum_scanrate;
        printf("Scan rate set to maximum %i ms\n", scanrate);
	}
	multiple = strtol (p2, &pp, 10);
	if (*pp) {
        printf("Epics rate must be an integer multiple %s\n", p2);
		return;
	}
	if (multiple < TcComms::minimum_multiple) {
		multiple = TcComms::minimum_multiple;
        printf("Multiple set to minimum %i \n", multiple);
	}
	if (multiple > TcComms::maximum_multiple) {
		multiple = TcComms::maximum_multiple;
        printf("Multiple set to maximum %i \n", multiple);
	}

	printf ("Scan rate is %i ms and epics update rate is %ix slower.\n", 
		scanrate, multiple);
    return;
}

/* List function to generate separate listings
   @brief channel lists
 ************************************************************************/
void tcList (const iocshArgBuf *args)
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
	tc_lists.push_back (make_tuple (p1, p2, nullptr, false));
}

/* Define a nick name or alias
   @brief alias
 ************************************************************************/
void tcAlias (const iocshArgBuf *args)
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
	tc_alias = p1;
}


/* Debugging function that prints the values for all records on the PLCs
   @brief TCat print vals
 ************************************************************************/
void tcPrintVals (const iocshArgBuf *args)
{
	plc::System::get().printVals();
	return;
}

/* Process hook
   @brief piniProcessHook
 ************************************************************************/
static void piniProcessHook(initHookState state)
{
    switch (state) {
    case initHookAtIocRun:
        break;

    case initHookAfterIocRunning:
		plc::System::get().set_ioc_state (true);
        break;

    case initHookAtIocPause:
		plc::System::get().set_ioc_state (false);
        break;

    case initHookAfterIocPaused:
        break;

	case initHookAfterFinishDevSup:
		break;

    default:
        break;
    }
}


/* Register functions to EPICS IOC shell
	@brief Register to iocsh
 ************************************************************************/
tcRegisterToIocShell::tcRegisterToIocShell () 
{
    iocshRegister(&tcLoadRecordsFuncDef, tcLoadRecords);
    iocshRegister(&tcSetScanRateFuncDef, tcSetScanRate);
    iocshRegister(&tcAliasFuncDef, tcAlias);
    iocshRegister(&tcListFuncDef, tcList);
	iocshRegister(&tcPrintValsFuncDef, tcPrintVals);
	initHookRegister(piniProcessHook);
}

// Static object to make sure opcRegisterToIocShell is called first off
tcRegisterToIocShell tcRegisterToIocShell::gtcRegisterToIocShell;

/** @} */

}

