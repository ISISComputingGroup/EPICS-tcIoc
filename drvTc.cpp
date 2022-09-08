#include "drvTc.h"
#include "ParseTpy.h"
#include "TpyToEpics.h"
#include "TpyToEpicsConst.h"
#include "infoPlc.h"
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

/** @file drvTc.cpp
	This contains functions for driver support for EPICS. These routines 
	are used during the IOC initialization, and allow the IOC to 
	initialize the TwinCAT interface during EPICS initialization.
 ************************************************************************/

static bool dbg = 0;


using namespace std;
using namespace ParseUtil;
using namespace ParseTpy;
using namespace EpicsTpy;

/** @defgroup iocshfunc Device driver functions
 ************************************************************************/
/** @{ */


namespace DevTc {

static const iocshArg tcLoadRecordsArg0	            = {"'tpy' Filename", iocshArgString};
static const iocshArg tcLoadRecordsArg1	            = {"Conversion rules", iocshArgString};
static const iocshArg tcSetScanRateArg0	            = {"TC scan rate in ms", iocshArgString};
static const iocshArg tcSetScanRateArg1	            = {"EPICS scan rate in multiples of the TC scan rate", iocshArgString};
static const iocshArg tcListArg0			        = {"'list' Filename", iocshArgString};
static const iocshArg tcListArg1		            = {"Conversion rules", iocshArgString};
static const iocshArg tcMacroArg0			        = {"'mdir' output directory", iocshArgString};
static const iocshArg tcMacroArg1		            = {"Macro arguments", iocshArgString};
static const iocshArg tcAliasArg0					= {"Alias name for PLC", iocshArgString};
static const iocshArg tcAliasArg1					= {"Replacement rules", iocshArgString};
static const iocshArg tcInfoPrefixArg0				= {"Prefix for info PLC records", iocshArgString};
static const iocshArg tcPrintValsArg0				= {"emptyarg", iocshArgString };
static const iocshArg tcPrintValArg0				= {"Variable name (accepts wildcards)", iocshArgString};

static const iocshArg* const  tcLoadRecordsArg[2]   = {&tcLoadRecordsArg0, &tcLoadRecordsArg1};
static const iocshArg* const  tcSetScanRateArg[2]   = {&tcSetScanRateArg0, &tcSetScanRateArg1};
static const iocshArg* const  tcListArg[2]		    = {&tcListArg0, &tcListArg1};
static const iocshArg* const  tcMacroArg[2]		    = {&tcMacroArg0, &tcMacroArg1};
static const iocshArg* const  tcAliasArg[2]			= {&tcAliasArg0, &tcAliasArg1};
static const iocshArg* const  tcInfoPrefixArg[1]	= {&tcInfoPrefixArg0};
static const iocshArg* const  tcPrintValsArg[1]		= {&tcPrintValsArg0};
static const iocshArg* const  tcPrintValArg[1]		= {&tcPrintValArg0};

static const iocshFuncDef tcLoadRecordsFuncDef      = {"tcLoadRecords", 2, tcLoadRecordsArg};
static const iocshFuncDef tcSetScanRateFuncDef	    = {"tcSetScanRate", 2, tcSetScanRateArg};
static const iocshFuncDef tcListFuncDef				= {"tcGenerateList", 2, tcListArg};
static const iocshFuncDef tcMacroFuncDef            = {"tcGenerateMacros", 2, tcMacroArg};
static const iocshFuncDef tcAliasFuncDef            = {"tcSetAlias", 2, tcAliasArg}; 
static const iocshFuncDef tcInfoPrefixFuncDef		= {"tcInfoPrefix", 1, tcInfoPrefixArg};
static const iocshFuncDef tcPrintValsFuncDef        = {"tcPrintVals", 1, tcPrintValsArg};
static const iocshFuncDef tcPrintValFuncDef			= {"tcPrintVal", 1, tcPrintValArg};

/// Tuple for filnemae, rule and list processing 
using filename_rule_list_tuple = 
	std::tuple<std::stringcase, std::stringcase, epics_list_processing*, bool>;
/// List of tuples for filnemae, rule and list processing 
using tc_listing_def = std::vector<filename_rule_list_tuple>;
/// Tuple for directory name, argument  and macro list processing
using dirname_arg_macro_tuple = 
	std::tuple<std::stringcase, std::stringcase, epics_macrofiles_processing*, const char*>;
/// List of tuples for directory name, argument  and macro list processing
using tc_macro_def = std::vector<dirname_arg_macro_tuple>;

static int scanrate = TcComms::default_scanrate;
static int multiple = TcComms::default_multiple;
static std::stringcase tc_alias;
static ParseUtil::replacement_rules tc_replacement_rules;
static tc_listing_def tc_lists;
static tc_macro_def tc_macros;
static std::stringcase tc_infoprefix;


/** Class for generating an EPICS database and tc record 
	@brief EPICS/TCat db processing
 ************************************************************************/
class epics_tc_db_processing : public EpicsTpy::epics_db_processing {
public:
	/// Default constructor
	/// @param p PLC
	/// @param rules Replacement rules
	/// @param l Pointer to list definitions
	/// @param m Pointer to macro definition
	explicit epics_tc_db_processing (TcComms::TcPLC& p,
		ParseUtil::replacement_rules& rules,
		tc_listing_def* l = nullptr, tc_macro_def* m = nullptr)
		: plc (&p), lists (l), macros (m) {
			device_support = device_support_type::tc_name;
#if EPICS_VERSION < 7
			int_support = int_support_type::int_32;
#endif
			set_rule_table(rules.get_rule_table());
			set_recursive(rules.is_recursive());
			init_lists(); init_macros(); }
	/// Destructor
	~epics_tc_db_processing() override { 
		done_lists(); done_macros(); }

	/// Parse a command line
	/// Processed options with epics_conversion::getopt and mygetopt.
	/// @param argc Number of command line arguments
	/// @param argv List of command line arguments, same format as in main()
	/// @param argp Excluded/processed arguments (in/out), array length must be argc
	/// @return Number of arguments processed
	int getopt(int argc, const char* const argv[], bool argp[] = 0) override;
	/// Set ignore for subsitution list on all list
	void set_ignore_all(bool ignr) noexcept;

	/// Process a variable
	/// @param arg Process argument describign the variable and type
	/// @return True if successful
	bool operator() (const ParseUtil::process_arg& arg) noexcept override;
	/// Flush output files
	void flush() noexcept;

	/// Get number of EPICS records without tc records
	int get_invalid_records() const noexcept { return invnum; }

protected:
	/// Disable copy constructor
	epics_tc_db_processing (const epics_tc_db_processing&) = delete;
	/// Disable move constructor
	epics_tc_db_processing(epics_tc_db_processing&&) = delete;
	/// Disable assignment operator
	epics_tc_db_processing& operator= (const epics_tc_db_processing&) = delete;
	/// Disable move assignment operator
	epics_tc_db_processing& operator= (epics_tc_db_processing&&) = delete;

	/// Init lists
	void init_lists() noexcept;
	/// Cleanup lists
	void done_lists() noexcept;
	/// Process all listings
	/// @param arg Process argument describign the variable and type
	/// @return True if successful
	bool process_lists (const ParseUtil::process_arg& arg) noexcept;
	/// Process a listing
	/// @param listdef filename/rule pair defining a listing
	/// @param arg Process argument describign the variable and type
	/// @return True if successful
	bool process_list (filename_rule_list_tuple& listdef, 
		const ParseUtil::process_arg& arg) noexcept;

	/// Init macros
	void init_macros() noexcept;
	/// Cleanup macros
	void done_macros() noexcept;
	/// Process all macros
	/// @param arg Process argument describign the variable and type
	/// @return True if successful
	bool process_macros (const ParseUtil::process_arg& arg) noexcept;
	/// Process a macro
	/// @param macrodef filename/rule pair defining a macro
	/// @param arg Process argument describign the variable and type
	/// @return True if successful
	bool process_macro (dirname_arg_macro_tuple& macrodef, 
		const ParseUtil::process_arg& arg) noexcept;

	/// Pointer to PLC class
	TcComms::TcPLC*		plc = nullptr;
	/// Pointer to a set of listings
	tc_listing_def*		lists = nullptr;
	/// Pointer to macros
	tc_macro_def*		macros = nullptr;
	/// Number of EPICS records without tc records
	int					invnum = 0;
};

/// @cond Doxygen_Suppress

/* epics_tc_db_processing::init_lists
 ************************************************************************/
void epics_tc_db_processing::init_lists() noexcept
{
	if (!lists) return;
	try {
		for (filename_rule_list_tuple& list : *lists) {
			optarg options(std::get<1>(list));
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
					std::stringcase arg(options.argv()[i]);
					// export only opc variables (default)
					if (arg == "-ns" || arg == "/ns") {
						no_string = true;
						options.argp()[i] = true;
					}
				}
				// option processing
				lproc->getopt(options.argc(), options.argv(), options.argp());
				// set replacement rules
				lproc->set_rule_table(get_rule_table());
				lproc->set_recursive(is_recursive());
				// force single file
				split_io_support iosupp(std::get<0>(list), false, 0);
				if (!iosupp) {
					printf("Failed to open output %s.\n", std::get<0>(list).c_str());
					delete lproc;
					lproc = nullptr;
				}
				else {
					(split_io_support&)(*lproc) = iosupp;
				}
			}
			if (std::get<epics_list_processing*>(list))
				delete std::get<epics_list_processing*>(list);
			std::get<epics_list_processing*>(list) = lproc;
			std::get<bool>(list) = no_string;
		}
	}
	catch (...) {}
}

/* epics_tc_db_processing::done_lists
 ************************************************************************/
void epics_tc_db_processing::done_lists() noexcept
{
	if (!lists) return;
	for (filename_rule_list_tuple& list : *lists) {
		if (std::get<epics_list_processing*>(list)) {
			delete std::get<epics_list_processing*>(list);
			std::get<epics_list_processing*>(list) = nullptr;
		}
	}
}

/* Process a channel
   epics_tc_db_processing::process_lists()
 ************************************************************************/
bool epics_tc_db_processing::process_lists (const ParseUtil::process_arg& arg) noexcept
{
	if (!lists) return true;
	bool succ = true;
	for (filename_rule_list_tuple& i : *lists) {
		if (!process_list (i, arg)) succ = false;
	}
	return succ;
}

/* Process a channel
   epics_tc_db_processing::process_list()
 ************************************************************************/
bool epics_tc_db_processing::process_list (filename_rule_list_tuple& listdef,
									  const ParseUtil::process_arg& arg) noexcept
{
	epics_list_processing* lptr = std::get<epics_list_processing*>(listdef);
	if (!lptr) return false;
	if (std::get<bool>(listdef) && arg.get_process_type() == process_type_enum::pt_string) return false;
	try {
		return (*lptr) (arg);
	}
	catch (...) {
		return false;
	}
}

/* epics_tc_db_processing::init_macros
 ************************************************************************/
void epics_tc_db_processing::init_macros() noexcept
{
	if (!macros) return;
	try {
		for (dirname_arg_macro_tuple& macro : *macros) {
			optarg options(std::get<1>(macro));
			epics_macrofiles_processing* mproc =
				new (std::nothrow) epics_macrofiles_processing(
					plc->get_alias(), std::get<0>(macro), false, options.argc(), options.argv());
			if (mproc) {
				// set replacement rules
				mproc->set_rule_table(get_rule_table());
				mproc->set_recursive(is_recursive());
				// set input directory to tpy file dir
				if (std::get<const char*>(macro) && *std::get<const char*>(macro)) {
					mproc->set_indirname(std::get<const char*>(macro));
				}
			}
			if (std::get<epics_macrofiles_processing*>(macro))
				delete std::get<epics_macrofiles_processing*>(macro);
			std::get<epics_macrofiles_processing*>(macro) = mproc;
		}
	}
	catch (...) {}
}

/* epics_tc_db_processing::done_macros
 ************************************************************************/
void epics_tc_db_processing::done_macros() noexcept
{
	if (!macros) return;
	for (dirname_arg_macro_tuple& list : *macros) {
		if (std::get<epics_macrofiles_processing*>(list)) {
			delete std::get<epics_macrofiles_processing*>(list);
			std::get<epics_macrofiles_processing*>(list) = nullptr;
		}
	}
}

/* Process a channel
   epics_tc_db_processing::process_macros()
 ************************************************************************/
bool epics_tc_db_processing::process_macros (const ParseUtil::process_arg& arg) noexcept
{
	if (!macros) return true;
	bool succ = true;
	for (dirname_arg_macro_tuple& i : *macros) {
		if (!process_macro (i, arg)) succ = false;
	}
	return succ;
}

/* Process a channel
   epics_tc_db_processing::process_macro()
 ************************************************************************/
bool epics_tc_db_processing::process_macro(dirname_arg_macro_tuple& macrodef,
									  const ParseUtil::process_arg& arg) noexcept
{
	epics_macrofiles_processing* mptr = std::get<epics_macrofiles_processing*>(macrodef);
	if (!mptr) return false;
	try {
		return (*mptr) (arg);
	}
	catch (...) {
		return false;
	}
}

/* Read options
   epics_tc_db_processing::getopt()
 ************************************************************************/
int epics_tc_db_processing::getopt(int argc, const char* const argv[], bool argp[])
{
	// call inherited getopt
	const int ret = EpicsTpy::epics_db_processing::getopt(argc, argv, argp);

	// copy substitution list into lists
	if (lists) {
		const substitution_list* r = this;
		for (filename_rule_list_tuple& list : *lists) {
			auto lproc = std::get<epics_list_processing*>(list);
			if (lproc) {
				// set substitution list
				substitution_list* l = lproc;
				*l = *r;
			}
		}
	}
	// copy substitution list into macros
	if (macros) {
		const substitution_list* r = this;
		for (dirname_arg_macro_tuple& macro : *macros) {
			auto mproc = std::get<epics_macrofiles_processing*>(macro);
			if (mproc) {
				// set substitution list
				substitution_list* l = mproc;
				*l = *r;
			}
		}
	}
	return ret;
}


/* Set ignore of substitutions on all lists
   epics_tc_db_processing::set_ignore_all()
 ************************************************************************/
void epics_tc_db_processing::set_ignore_all(bool ignr) noexcept
{
	ignore = ignr; // database 
	// Set ignore of substitutions into lists
	if (lists) {
		for (filename_rule_list_tuple& list : *lists) {
			auto lproc = std::get<epics_list_processing*>(list);
			if (lproc) {
				lproc->set_ignore(ignr);
			}
		}
	}
	// Set ignore of substitutions into macros
	if (macros) {
		for (dirname_arg_macro_tuple& macro : *macros) {
			auto mproc = std::get<epics_macrofiles_processing*>(macro);
			if (mproc) {
				mproc->set_ignore(ignr);
			}
		}
	}
}

/* Process a channel
   epics_tc_db_processing::operator()
 ************************************************************************/
bool epics_tc_db_processing::operator() (const ParseUtil::process_arg& arg) noexcept
{
	// Generate EPICS database record
	if (!EpicsTpy::epics_db_processing::operator()(arg)) {
		// These are structs!
		process_macros (arg); // need to process binaries!
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
	else if (arg.get_type_name() == "INT" || arg.get_process_type() == process_type_enum::pt_enum)
		rt = plc::data_type_enum::dtInt16;
	else if (arg.get_type_name() == "UINT" || arg.get_type_name() =="WORD") 
		rt = plc::data_type_enum::dtUInt16;
	else if (arg.get_type_name() == "DINT") 
		rt = plc::data_type_enum::dtInt32;
	else if (arg.get_type_name() == "UDINT" || arg.get_type_name() == "DWORD" ||
		     arg.get_type_name() == "TIME"  || arg.get_type_name() == "TOD"   || 
			 arg.get_type_name() == "DATE"  || arg.get_type_name() == "DT"    || 
		     arg.get_type_name() == "TIME_OF_DAY" || arg.get_type_name() == "DATE_AND_TIME")
		rt = plc::data_type_enum::dtUInt32;
	else if (arg.get_type_name() == "LINT")
		rt = plc::data_type_enum::dtInt64;
	else if (arg.get_type_name() == "ULINT" || arg.get_type_name() == "LWORD" || 
		     arg.get_type_name() == "LTIME")
		rt = plc::data_type_enum::dtUInt64;
	else if (arg.get_type_name() == "REAL")
		rt = plc::data_type_enum::dtFloat;
	else if (arg.get_type_name() == "LREAL") 
		rt = plc::data_type_enum::dtDouble;
	else if (arg.get_type_name().substr(0,6) == "STRING") 
		rt = plc::data_type_enum::dtString;
	else {
		printf ("Unknown type %s for %s\n", arg.get_type_name().c_str(), arg.get_name().c_str());
		++invnum;
		return false;
	}

	try {
		/// Make new record object
		plc::BaseRecordPtr pRecord = plc::BaseRecordPtr(new plc::BaseRecord(arg.get_full(), rt));
		plc::Interface* iface = nullptr;

		/// Make TCat interface
		const process_arg_tc* targ = dynamic_cast<const process_arg_tc*>(&arg);
		if (targ) {
			std::stringcase tcatname = arg.get_alias();
			if (HasRules()) {
				tcatname = apply_replacement_rules (tcatname);
			}
			TcComms::TCatInterface* tcat = new (std::nothrow) TcComms::TCatInterface (*pRecord,
				tcatname,
				targ->get_igroup(),
				targ->get_ioffset(),
				targ->get_bytesize(),
				arg.get_type_name(),
				arg.get_process_type() == process_type_enum::pt_binary,
				arg.get_process_type() == process_type_enum::pt_enum);
			iface = tcat;
		}

		/// Make info interface
		else {
			std::stringcase tcatname = arg.get_alias();
			if (HasRules()) {
				tcatname = apply_replacement_rules(tcatname);
			}
			const InfoPlc::process_arg_info* iarg = dynamic_cast<const InfoPlc::process_arg_info*>(&arg);
			if (iarg) {
				InfoPlc::InfoInterface* info = new (std::nothrow) InfoPlc::InfoInterface(*pRecord,
					arg.get_name(), tcatname, arg.get_type_name());
				iface = info;
			}
		}

		if (iface == 0) {
			// this means the allocation failed!
			++invnum;
			return false;
		}

		/// Tell record about TCat interface
		pRecord->set_plcInterface (iface);

		if (!plc->add (pRecord)) {
			// this means the EPICS record has nothing to connect to!
			++invnum;
			return false;
		}

		process_lists(arg);
		process_macros(arg);
		return true;
	}
	catch (...) {
		return false;
	}
}


/* Flush output files
   epics_tc_db_processing::flush()
 ************************************************************************/
void epics_tc_db_processing::flush() noexcept
{
	EpicsTpy::epics_db_processing::flush();
	if (lists) {
		for (filename_rule_list_tuple& list : *lists) {
			if (std::get<epics_list_processing*>(list)) 
				std::get<epics_list_processing*>(list)->flush();
		}
	}
	if (macros) {
		for (dirname_arg_macro_tuple& macro : *macros) {
			if (std::get<epics_macrofiles_processing*>(macro)) 
				std::get<epics_macrofiles_processing*>(macro)->flush();
		}
	}
}

/// @endcond

/** Function for loading a TCat tpy file, and using it to generate 
	internal record entries as well as the EPICs .db file
	@brief Load TwinCAT records
	@param args Arguments for tcLoadRecords
 ************************************************************************/
void tcLoadRecords (const iocshArgBuf *args) 
{
	// save and reset alias name, listings and macro
	std::stringcase alias = tc_alias;
	ParseUtil::replacement_rules rules = tc_replacement_rules;
	tc_listing_def listings = tc_lists;
	tc_macro_def macros = tc_macros;
	std::stringcase infoprefix = tc_infoprefix;
	tc_alias = "";
	tc_replacement_rules.clear();
	tc_lists.clear();
	tc_macros.clear();
	tc_infoprefix = "";

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
	if (fopen_s(&inpf, args[0].sval, "r")) {
		printf ("Failed to open input %s.\n", args[0].sval);
		return;
	}
	for (dirname_arg_macro_tuple& macro : macros) {
		std::get<const char*>(macro) = args[0].sval;
	}
	
	// check option arguments
	optarg options;
	if (args[1].sval) {
		options.parse (args[1].sval);
	}

	// Timer for just tpy file parsing
	clock_t tpybegin = 0;
	clock_t tpyend = 0;

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
	const int port = tpyfile.get_project_info().get_port();

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
	try {
		epics_tc_db_processing dbproc(*tcplc, rules, &listings, &macros);
		// option processing
		dbproc.getopt(options.argc(), options.argv(), options.argp());
		// force single file
		split_io_support iosupp(outfilename, false, 0);
		if (!iosupp) {
			printf("Failed to open output %s.\n", outfilename.c_str());
			return;
		}
		(split_io_support&)(dbproc) = iosupp;
		// setup macro processing
		for (dirname_arg_macro_tuple& macro : macros) {
			if (std::get<epics_macrofiles_processing*>(macro)) {
				const bool twincat3 = (tpyfile.get_project_info().get_tcat_version_major() >= 3);
				std::get<epics_macrofiles_processing*>(macro)->set_twincat3(twincat3);
				// set output dir to the input dir for Tc3
				if (twincat3) std::get<epics_macrofiles_processing*>(macro)->set_indirname(
					std::get<epics_macrofiles_processing*>(macro)->get_outdirname());
			}
		}

		// generate db file from tc records
		if (dbg) tpyfile.set_export_all(TRUE);
		int num = tpyfile.process_symbols(dbproc);

		// generate db file from info records
		if (!infoprefix.empty()) {
			dbproc.flush();
			const bool save_ignore = dbproc.get_ignore();
			dbproc.set_ignore_all(true);
			num += InfoPlc::InfoInterface::get_infodb(infoprefix,
				tpyfile.get_project_info().get(), dbproc);
			dbproc.set_ignore_all(save_ignore);
		}

		// make sure all file contents is written to file
		dbproc.flush();

		// check unused entries in the substitution list
		dbproc.check_unused_subsititions();
		// write statistics
		if (dbproc.get_invalid_records() == 0) {
			printf("Loaded %i records from %s.\n", num, args[0].sval);
		}
		else {
			printf("Loaded %i valid and %i invaid records from %s.\n",
				num, dbproc.get_invalid_records(), args[0].sval);
		}
	}
	catch (...) {
		printf("Failed to generate database file %s.\n", outfilename.c_str());
		return;
	}

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
#ifdef DEBUG
		return;
#endif
	}

	plc::System::get().add(plc::BasePLCPtr(tcplc)); // adopted by TSystem

	// load epics database
	//pos = outfilename.rfind ('\\');
	//stringcase path;
	//stringcase fname;
	//if (pos == stringcase::npos) {
	//	fname = outfilename;
	//}
	//else {
	//	fname = outfilename.substr (pos + 1);
	//	path  = outfilename.substr (0, pos);
	//}

	printf ("Loading record database %s.\n", outfilename.c_str());
	if (dbLoadRecords (outfilename.c_str(), 0)) {
		printf ("\nUnable to load record database for %s.\n", outfilename.c_str());
		return;
	}
	printf ("Loaded record database %s.\n", outfilename.c_str());
	// success!

	return;
}

/** Set scan rate of the read scanner
	@brief Set the scan rate
 	@param args Arguments for tcSetScanRate
************************************************************************/
void tcSetScanRate (const iocshArgBuf *args) noexcept
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

/** List function to generate separate listings
    @brief Generate channel lists
	@param args Arguments for tcList
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

/** Macro function to generate macro files
    @brief Generated macro files
 	@param args Arguments for tcMacro
************************************************************************/
void tcMacro (const iocshArgBuf *args)
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
	const char* p2 = args[1].sval ? args[1].sval : "";
	if (!p1) {
        printf("Specify an output directory for the macro files\n");
		return;
	}
	tc_macros.push_back (dirname_arg_macro_tuple (p1, p2, nullptr, nullptr));
}

/** Define a nick name or alias
    @brief Define alias and replacement rules
	@param args Arguments for tcAlias
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
	// Check alias name
	const char* p1 = args[0].sval;
	if (!p1) {
        printf("Specify an alias name for a tc PLC\n");
		return;
	}
	tc_alias = p1;
	// Check replacement rules
	tc_replacement_rules.clear();
	const char* p2 = args[1].sval;
	if (p2) {
		tc_replacement_rules.parse_rules(p2, tc_alias);
		std::stringcase msg = "Replacement rules are: " + tc_replacement_rules.get_rule_string();
		printf("%s\n", msg.c_str());
	}
}


/** Sets the channel prefix for info PLC records
	@brief Sets the info prefix
 	@param args Arguments for tcInfoPrefix
************************************************************************/
void tcInfoPrefix(const iocshArgBuf *args)
{
	// Check if Ioc is running
	if (plc::System::get().is_ioc_running()) {
		printf("IOC is already initialized\n");
		return;
	}
	// Check arguments
	if (!args) {
		printf("Specify a info prefix\n");
		return;
	}
	// Check prefix string
	const char* p1 = args[0].sval;
	if (!p1) {
		printf("Specify an info prefix for a tc PLC\n");
		return;
	}
	tc_infoprefix = p1;
	// Unescape \$
	stringcase::size_type pos;
	while ((pos = tc_infoprefix.find("\\$")) != stringcase::npos) {
		tc_infoprefix.erase(pos, 1);
	}
	return;
}

/** Debugging function that prints the values for all records of the PLCs
	@brief Print all values
 	@param args Arguments for tcPrintVals
************************************************************************/
void tcPrintVals(const iocshArgBuf *args)
{
	plc::System::get().printVals();
	return;
}

/** Debugging function that prints the values for one or multiple records 
	of the PLCs. Supports wildcards.
	@brief Print value
	@param args Arguments for tcPrintVal
 ************************************************************************/
void tcPrintVal (const iocshArgBuf *args)
{
	// Check argument string
	const char* p1 = args[0].sval;
	if (!p1) {
		printf("Specify variable name or regex\n");
		return;
	}

	plc::System::get().printVal (p1);
	return;
}

/*  Process hook
    @brief piniProcessHook
 ************************************************************************/

#pragma warning (disable : 26812)
static void piniProcessHook (initHookState state) noexcept
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
#pragma warning (default : 26812)


/** Register functions to EPICS IOC shell
	@brief Register to iocsh
 ************************************************************************/
tcRegisterToIocShell::tcRegisterToIocShell () noexcept
{
    iocshRegister(&tcLoadRecordsFuncDef, tcLoadRecords);
    iocshRegister(&tcSetScanRateFuncDef, tcSetScanRate);
    iocshRegister(&tcAliasFuncDef, tcAlias);
    iocshRegister(&tcListFuncDef, tcList);
    iocshRegister(&tcMacroFuncDef, tcMacro);
	iocshRegister(&tcInfoPrefixFuncDef, tcInfoPrefix);
	iocshRegister(&tcPrintValsFuncDef, tcPrintVals);
	iocshRegister(&tcPrintValFuncDef, tcPrintVal);
	initHookRegister(piniProcessHook);
}

// Static object to make sure opcRegisterToIocShell is called first off
tcRegisterToIocShell tcRegisterToIocShell::gtcRegisterToIocShell;

}

/** @} */

