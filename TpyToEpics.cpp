#include "stdafx.h"
#include "ParseUtilConst.h"
#include "ParseTpyConst.h"
#include "ParseTpy.h"
#include "TpyToEpicsConst.h"
#include "TpyToEpics.h"
#include <filesystem>

using namespace std;
using namespace std::filesystem;
using namespace ParseTpy;
using namespace ParseUtil;


/** @file TpyToEpics.cpp
	Source for methods that generate EPICs .db file from a .tpy file
 ************************************************************************/

namespace EpicsTpy {


/* Parse command line arguments
 ************************************************************************/
int epics_conversion::getopt (int argc, const char* const argv[], bool argp[])
{
	int num = 0;
	for (int i = 1; i < argc; ++i) {
		if (argp && argp[i]) continue;
		if (!argv[i]) {
			if (argp) argp[i] = true;
			continue;
		}
		std::stringcase arg (argv[i]);
		int oldnum = num;
		// Does not apply any special conversion rules
		if (arg == "-rn" || arg == "/rn" ) {
			set_conversion_rule (tc_epics_conv::no_conversion);
			++num;
		}
		// Replaces dots with underscores in channel names
		else if (arg == "-rd" || arg == "/rd" ) {
			set_conversion_rule (tc_epics_conv::no_dot);
			++num;
		}
		// LIGO standard conversion rule (default)
		else if (arg == "-rl" || arg == "/rl" ) {
			set_conversion_rule (tc_epics_conv::ligo_std);
			++num;
		}
		// LIGO standard conversion rule for vacuum channels
		else if (arg == "-rv" || arg == "/rv" ) {
			set_conversion_rule (tc_epics_conv::ligo_vac);
			++num;
		}
		// Preserve case in EPICS channel names (default)
		else if (arg == "-cp" || arg == "/cp" ) {
			set_case_rule (case_type::preserve);
			++num;
		}
		// Force upper case in EPICS channel names
		else if (arg == "-cu" || arg == "/cu" ) {
			set_case_rule (case_type::upper);
			++num;
		}
		// Force lower case in EPICS channel names
		else if (arg == "-cl" || arg == "/cl" ) {
			set_case_rule (case_type::lower);
			++num;
		}
		// Eliminates leading dot in channel name
		else if (arg == "-nd" || arg == "/nd") {
			set_dot_rule (true);
			++num;
		}
		// Leaves leading dot in channel name (default)
		else if (arg == "-yd" || arg == "/yd") {
			set_dot_rule (false);
			++num;
		}
		// Replaces array brackets with underscore
		else if (arg == "-ni" || arg == "/ni") {
			set_array_rule (true);
			++num;
		}
		// Leave array indices as is (default)
		else if (arg == "-yi" || arg == "/yi") {
			set_array_rule (false);
			++num;
		} 
		// Check if a prefix has been specified
		else if ((arg == "-p" || arg == "/p") && i + 1 < argc) {
			set_prefix (i + 1 < argc && argv[i+1] ? argv[i+1] : "");
			i += 1;
			++num;
		}
		// Check if a prefix has been specified
		else if ((arg == "-p" || arg == "/p") && i + 1 < argc) {
			set_prefix (i + 1 < argc && argv[i + 1] ? argv[i + 1] : "");
			if (argp) argp[i] = true;
			i += 1;
			num += 2;
		}
		// now set flag to indicated a processed option
		if (argp && (num > oldnum)) {
			argp[i] = true;
		}
	}
	return num;
}

/* Obtain EPICS name from OPC name
 ************************************************************************/
string epics_conversion::to_epics (const stringcase& name) const
{
	stringcase n (name);
	stringcase::size_type pos;

	// apply replacement rules
	if (HasRules()) {
		n = apply_replacement_rules (n);
	}

	// eliminate leading dot
	if (no_leading_dot || (conv_rule == tc_epics_conv::ligo_std) || (conv_rule == tc_epics_conv::ligo_vac)) {
		std::stringcase::size_type pos;
		if (!n.empty() && (n[0] == '.')) {
			n.erase (0, 1);
		}
		// TwinCAT 3 doesn't use an empty name for globals
		else if ((pos = n.find ('.')) != std::stringcase::npos) {
			n.erase (0, pos + 1);
		}
	}

	// apply conversion rules
	switch (conv_rule) {
		// ligo standard
	case tc_epics_conv::ligo_std:
		// replace first dot with colon
		pos = n.find ('.');
		if (pos != stringcase::npos) {
			n[pos] = ':';
		}
		// replace second dot with dash
		pos = n.find ('.');
		if (pos != stringcase::npos) {
			n[pos] = '-';
		}
		// replace remaining dots with underscore
		while ((pos = n.find ('.')) != stringcase::npos) {
			n[pos] = '_';
		}
		break;
		// ligo standard
	case tc_epics_conv::ligo_vac:
		// replace first underscore with colon
		pos = n.find ('_');
		if (pos != stringcase::npos) {
			n[pos] = ':';
		}
		// replace second underscore with dash
		pos = n.find ('_');
		if (pos != stringcase::npos) {
			n[pos] = '-';
		}
		// replace dots with underscore
		while ((pos = n.find ('.')) != stringcase::npos) {
			n[pos] = '_';
		}
		break;
		// replace all dots with underscores
	case tc_epics_conv::no_dot:
		while ((pos = n.find ('.')) != stringcase::npos) {
			n[pos] = '_';
		}
		break;
		// do nothing
	case tc_epics_conv::no_conversion:
	default:
		break;
	}
	// force case if necessary
	if (case_epics_names != case_type::preserve) {
		for (pos = 0; pos < n.size(); ++pos) {
			n[pos] = (case_epics_names == case_type::upper) ? toupper (n[pos]) : tolower (n[pos]);
		}
	}
	// replace array brackets with underscore if necessary
	if (no_array_index) {
		while ((pos = n.find ('[')) != stringcase::npos) {
			n[pos] = '_';
		}
		while ((pos = n.find (']')) != stringcase::npos) {
			n.erase (pos, 1);
		}
	}
	// add prefix
	n = get_prefix() + n;

	return string (n.c_str());
}


/* Destructor
split_io_support::~split_io_support
************************************************************************/
split_io_support::~split_io_support()
{
	close();
}

/* Copy constructor
split_io_support::~split_io_support
************************************************************************/
split_io_support::split_io_support (const split_io_support& iosup)
	: error(true), split_io(false), split_n(0), outf(0), 
	outf_in(nullptr), outf_io(nullptr), rec_num(0), rec_num_in(0), 
	rec_num_io(0), file_num_in(1), file_num_io(1)
{
	*this = iosup;
}

/* Assignment operator
split_io_support::~split_io_support
************************************************************************/
split_io_support& split_io_support::operator= (const split_io_support& iosup)
{
	close();
	error = iosup.error;
	outfilename = iosup.outfilename;
	split_io = iosup.split_io;
	split_n = iosup.split_n;
	rec_num = iosup.rec_num;
	rec_num_in = iosup.rec_num_in;
	rec_num_io = iosup.rec_num_io;
	file_num_in = iosup.file_num_in;
	file_num_io = iosup.file_num_io;
	file_num_in_s = iosup.file_num_in_s;
	file_num_io_s = iosup.file_num_io_s;
	file_in_s = iosup.file_in_s;
	file_io_s = iosup.file_io_s;
	outf = iosup.outf;
	outf_in = iosup.outf_in;
	outf_io = iosup.outf_io;
	iosup.outf = nullptr;
	iosup.outf_in = nullptr;
	iosup.outf_io = nullptr;
	return *this;
}

/* Parse command line arguments
 ************************************************************************/
int split_io_support::getopt (int argc, const char* const argv[], bool argp[])
{
	int num = 0;
	for (int i = 1; i < argc; ++i) {
		if (argp && argp[i]) continue;
		if (!argv[i]) {
			if (argp) argp[i] = true;
			continue;
		}
		std::stringcase arg (argv[i]);
		int oldnum = num;
		// Splits database into input only and input/ouput recrods 
		if (arg == "-sio" || arg == "/sio" ||
			arg == "-ysio" || arg == "/ysio") {
			set_split (true);
			++num;
		}
		// Does not split database by record type (default)
		else if (arg == "-nsio" || arg == "/nsio" ) {
			set_split (false);
			++num;
		}
		// Splits database into files with no more than num records
		else if (arg == "-sn" || arg == "/sn" ) {
			if  (i + 1 < argc && argv[i+1][0] != '/' && argv[i+1][0] != '-') {
				int n = atoi (argv[i+1]);
				set_max (n);
				if (argp) argp[i] = true;
				i += 1;
				num += 2;
			}
			else {
				++num;
			}
		}
		// no set flag to indicated a processed option
		if (argp && (num > oldnum)) {
			argp[i] = true;
		}
	}
	return num;
}

/* Set the file name
   split_io_support::set_filename
************************************************************************/
void split_io_support::set_filename (const stringcase& fn)
{
	close();
	outfilename = fn;
	// check for non-empty (no stdout) filename
	if (!outfilename.empty()) {
		if (split_io || (split_n > 0)) {
			if (outfilename.rfind (".db") == outfilename.size() - 3) {
				outfilename.erase (outfilename.size() - 3);
			}
			if (split_n > 0) {
				char buf[20];
				sprintf_s (buf, sizeof (buf), ".%03i", file_num_in);
				file_num_in_s = buf;
				sprintf_s (buf, sizeof (buf), ".%03i", file_num_io);
				file_num_io_s = buf;
			}
			if (split_io) {
				file_in_s = ".in";
				file_io_s = ".io";
			}
			stringcase fname (outfilename + file_io_s + file_num_io_s + ".db");
			#pragma warning (disable: 4996)
			outf_io = fopen (fname.c_str(), "w");
			#pragma warning (default: 4996)
			if (!outf_io) {
				fprintf (stderr, "Failed to open output %s.\n", fname.c_str());
				error = true;
				return;
			}
			outf = outf_io;
			if (split_io) {
				fname = outfilename + file_in_s + file_num_in_s + ".db";
				#pragma warning (disable: 4996)
				outf_in = fopen (fname.c_str(), "w");
				#pragma warning (default: 4996)
				if (!outf_io) {
					fprintf (stderr, "Failed to open output %s.\n", fname.c_str());
					error = true;
					return;
				}
			}
		}
		else {
			#pragma warning (disable: 4996)
			outf_io = fopen (outfilename.c_str(), "w");
			#pragma warning (default: 4996)
			if (!outf_io) {
				fprintf (stderr, "Failed to open output %s.\n", outfilename.c_str());
				error = true;
				return;
			}
			outf = outf_io;
		}
	}

	// empty string is stdout
	else {
		if (split_io || (split_n > 0)) {
			fprintf (stderr, "Cannot split output to console\n");
			error = true;
			return;
		}
		outf = stdout;
	}
}

/* Flush file content
   split_io_support::flush
************************************************************************/
void split_io_support::flush()
{
	if (outf_io) fflush (outf_io);
	if (outf_in) fflush (outf_in);
}

/* Closes files
   split_io_support::close
************************************************************************/
void split_io_support::close()
{
	if (outf_io) fclose (outf_io);
	outf_io = nullptr;
	if (outf_in) fclose (outf_in);
	outf_in = nullptr;
}

/* Increment record  number
   split_io_support::increment
************************************************************************/
bool split_io_support::increment (bool readonly)
{
	// check if we need to open a new split file
	if (!error && (split_n > 0)) {
		if (split_io) {
			if (readonly) {
				if ((rec_num_in > 0) && (rec_num_in % split_n == 0) && outf_in) {
					++file_num_in;
					char buf[20];
					sprintf_s (buf, sizeof(buf), ".%03i", file_num_in);
					file_num_in_s = buf;
					fclose (outf_in);
					stringcase fname = outfilename + file_in_s + file_num_in_s + ".db";
					#pragma warning (disable: 4996)
					outf_in = fopen (fname.c_str(), "w");
					#pragma warning (default: 4996)
					if (!outf_in) {
						fprintf (stderr, "Failed to open output %s.\n", fname.c_str());
						error = true;
					}
				}
			}
			else {
				if ((rec_num_io > 0) && (rec_num_io % split_n == 0) && outf_io) {
					++file_num_io;
					char buf[20];
					sprintf_s (buf, sizeof(buf), ".%03i", file_num_io);
					file_num_io_s = buf;
					fclose (outf_io);
					stringcase fname (outfilename + file_io_s + file_num_io_s + ".db");
					#pragma warning (disable: 4996)
					outf_io = fopen (fname.c_str(), "w");
					#pragma warning (default: 4996)
					if (!outf_io) {
						fprintf (stderr, "Failed to open output %s.\n", fname.c_str());
						error = true;
					}
				}
			}
		}
		// no splitting
		else {
			if ((rec_num > 0) && (rec_num % split_n == 0) && outf_io) {
				++file_num_io;
				char buf[20];
				snprintf (buf, sizeof (buf), ".%03i", file_num_io);
				file_num_io_s = buf;
				fclose (outf_io);
				stringcase fname (outfilename + file_io_s + file_num_io_s + ".db");
				#pragma warning (disable: 4996)
				outf_io = fopen (fname.c_str(), "w");
				#pragma warning (default: 4996)
				if (!outf_io) {
					fprintf (stderr, "Failed to open output %s.\n", fname.c_str());
					error = true;
				}
			}
		}
	}
	// set up output file
	if (split_io && readonly) outf = outf_in;
	else if (!outfilename.empty()) outf = outf_io;
	if (!outf) {
		outf = stdout;
		error = true;
	}
	// increase record number
	if (!error) {
		if (readonly) ++rec_num_in;
		else ++rec_num_io;
		++rec_num;
	}

	return error;
}


/* Open a file
   multi_io_support::operator
************************************************************************/
bool multi_io_support::operator! () const
{ 
	path opath (outdirname.c_str());
	path ipath (indirname.c_str());
	return !(is_directory (opath) && is_directory (ipath));
}

/* Open a file
   multi_io_support::open
************************************************************************/
bool multi_io_support::open (const std::stringcase& fname, 
							 const std::stringcase& io, bool superrmsg)
{
	close();
	if (io.find ('r') != stringcase::npos) {
		filestat = io_filestat::read;
	}
	else {
		filestat = io_filestat::write;
	}
	path newfile ((filestat == io_filestat::read ? indirname : outdirname).c_str());
	newfile /= fname.c_str();
	#pragma warning (disable: 4996)
	FILE* fio = fopen (newfile.string().c_str(), io.c_str());
	#pragma warning (default: 4996)
	if (!fio) {
		filestat = io_filestat::closed;
		if (!superrmsg) {
			fprintf (stderr, "Failed to open %s.\n", newfile.string().c_str());
		}
		return false;
	}
	filename = newfile.string().c_str();
	filehandle = fio;
	if (filestat == io_filestat::read) {
		file_num_in += 1;
	}
	else {
		file_num_out += 1;
	}
	//fprintf (stderr, "Opening %s for %s\n", filename.c_str(), 
	//	filestat == io_filestat::read ? "reading" : "writing");

	return true;
}

/* Closes file
   multi_io_support::close
************************************************************************/
void multi_io_support::close()
{
	if (filehandle) fclose (filehandle);
	filehandle = 0;
	filestat = io_filestat::closed;
}

/* Set input directory name
   multi_io_support::set_indirname
************************************************************************/
void multi_io_support::set_indirname (const std::stringcase& dname) 
{
	path fname (dname.c_str());
	if (is_directory (fname)) {
		indirname = fname.string().c_str();
	}
	else {
		indirname = fname.parent_path().string().c_str();
	}
}

/* Set output directory name
   multi_io_support::set_outdirname
************************************************************************/
void multi_io_support::set_outdirname (const std::stringcase& dname) 
{
	path fname (dname.c_str());
	outdirname = fname.string().c_str();
	try {
		create_directories (fname);
	}
	catch (...) {}
}


/* Option processing
   epics_list_processing::epics_list_processing
************************************************************************/
epics_list_processing::epics_list_processing (
		const std::stringcase& fname, 
		int argc, const char* const argv[], bool argp[])
	: epics_conversion (argc, argv, argp), 
	  split_io_support (fname, argc, argv, argp), 
	  listing(listing_type::standard), verbose(false)
{
	mygetopt (argc, argv, argp); 
}

/* Option processing
   epics_list_processing::getopt
************************************************************************/
int epics_list_processing::getopt (int argc, const char* const argv[], 
								   bool argp[]) 
{
	return epics_conversion::getopt (argc, argv, argp) +
		   mygetopt (argc, argv, argp); 
}

/* Option processing
   epics_list_processing::mygetopt
************************************************************************/
int epics_list_processing::mygetopt (int argc, const char* const argv[], 
								     bool argp[]) 
{
	int num = 0;
	for (int i = 1; i < argc; ++i) {
		if (argp && argp[i]) continue;
		if (!argv[i]) {
			if (argp) argp[i] = true;
			continue;
		}
		std::stringcase arg (argv[i]);
		int oldnum = num;
		// generate a standard listing
		if (arg == "-l" || arg == "/l") {
			set_listing (listing_type::standard);
			set_verbose (false);
			++num;
		}
		// generate a standard long listing
		else if (arg == "-ll" || arg == "/ll") {
			set_listing (listing_type::standard);
			set_verbose (true);
			++num;
		}
		// generate a burt save/restore listing
		else if (arg == "-lb" || arg == "/lb") {
			set_listing (listing_type::autoburt);
			set_verbose (false);
			++num;
		}
		// generate a LIGO DAQ ini listing
		else if (arg == "-li" || arg == "/li") {
			set_listing(listing_type::daqini);
			set_verbose(false);
			++num;
		}
		// now set flag to indicated a processed option
		if (argp && (num > oldnum)) {
			argp[i] = true;
		}
	}
	return num;
}

/* Process a channel
   epics_list_processing::operator()
************************************************************************/
bool epics_list_processing::operator() (const process_arg& arg)
{
	// quit if not atomic and not a standard listing
	if (!arg.is_atomic() && (listing != listing_type::standard)) {
		return false;
	}

	increment (arg.get_opc().is_readonly());
	// write record information to output file
	// produce a listing
	string epicsname = to_epics (arg.get_alias());
	if (listing == listing_type::autoburt) {
		stringcase ro = arg.get_opc().is_readonly() ? "RO " : "";
		fprintf (get_file(), "%s%s", ro.c_str(), epicsname.c_str());
	}
	// LIGO DAQ ini listing
	else if (listing == listing_type::daqini) {
		// get unit string and datatype value
		// we support float (4) and int32 (3)
		std::stringcase s;
		std::stringcase sep;
		std::stringcase unit;
		int datatype = LIGODAQ_DATATYPE_FLOAT;
		arg.get_opc().get_property(OPC_PROP_UNIT, unit);
		trim_space(unit);
		switch (arg.get_process_type()) {
		case process_type_enum::pt_int:
			datatype = LIGODAQ_DATATYPE_INT32;
			break;
		case process_type_enum::pt_bool:
			datatype = LIGODAQ_DATATYPE_INT32;
			if (arg.get_opc().get_property(OPC_PROP_OPEN, s)) {
				trim_space(s);
				unit = s;
			}
			unit += '|';
			if (arg.get_opc().get_property(OPC_PROP_CLOSE, s)) {
				trim_space(s);
				unit += s;
			}
			break;
		case process_type_enum::pt_enum:
			datatype = LIGODAQ_DATATYPE_INT32;
			unit = "";
			sep = "";
			for (int opcidx = OPC_PROP_ZRST; opcidx <= OPC_PROP_FFST; opcidx++) {
				if (arg.get_opc().get_property(opcidx, s)) {
					trim_space(s);
					unit += sep + s;
					sep = "";
				}
				sep += '|';
			}
			break;
		}
		if (unit == "") {
			unit = LIGODAQ_UNIT_NONE;
		}
		// write header 
		if (get_processed_total() == 1) {
			fprintf(get_file(), LIGODAQ_INI_HEADER, LIGODAQ_DATATYPE_DEFAULT, LIGODAQ_UNIT_DEFAULT);
			fprintf(get_file(), "\n\n");
		}
		// write entry for channel
		fprintf(get_file(), "[%s]", epicsname.c_str());
		if (datatype != LIGODAQ_DATATYPE_DEFAULT) {
			fprintf(get_file(), "\n%s=%i", LIGODAQ_DATATYPE_NAME, datatype);
		}
		s = unit;
		unit = "";
		for (const auto& c : s) {
			unsigned char uc = c;
			if (isprint(uc)) {
				unit += isspace(uc) ? '_' : uc;
			}
		}
		if (unit != LIGODAQ_UNIT_DEFAULT) {
			fprintf(get_file(), "\n%s=%s", LIGODAQ_UNIT_NAME, unit.c_str());
		}
	}
	// standard listing
	else {
		fprintf (get_file(), "%s", epicsname.c_str());
	}

	// long listing?
	if (verbose && (listing != listing_type::autoburt) && (listing != listing_type::daqini)) {
		fprintf (get_file(), " (%s", arg.get_process_string().c_str());
		fprintf (get_file(), ", opc %c", arg.get_opc().is_published() ? '1' : '0');
		for (const auto& i : arg.get_opc().get_properties()) {
				stringcase s = i.second;
				trim_space (s);
				fprintf (get_file(), ", prop[%i]=\"%s\"", i.first, s.c_str());
		}
		fprintf (get_file(), ")");
	}
	fprintf (get_file(), "\n");
	return true;
}


/* Option processing
   epics_macrofiles_processing::epics_macrofiles_processing
************************************************************************/
epics_macrofiles_processing::epics_macrofiles_processing (
		const std::stringcase& pname, const std::stringcase& dname, bool tcat3,
		int argc, const char* const argv[], bool argp[])
	: epics_conversion (argc, argv, argp), 
	  multi_io_support (dname, argc, argv, argp),
	   plcname (pname), macros (macrofile_type::all), isTwinCAT3 (tcat3), rec_num (0)
{
	mygetopt (argc, argv, argp); 
}


/* epics_macrofiles_processing::flush
************************************************************************/
void epics_macrofiles_processing::flush()
{
	while (!procstack.empty()) {
		process_record (procstack.top());
		procstack.pop();
	}
	fflush (stderr);
}

/* Option processing
   epics_macrofiles_processing::getopt
************************************************************************/
int epics_macrofiles_processing::getopt (
	int argc, const char* const argv[], bool argp[]) 
{
	return epics_conversion::getopt (argc, argv, argp) +
		   mygetopt (argc, argv, argp); 
}

/* Option processing
   epics_macrofiles_processing::mygetopt
************************************************************************/
int epics_macrofiles_processing::mygetopt (
	int argc, const char* const argv[], bool argp[]) 
{
	int num = 0;
	for (int i = 1; i < argc; ++i) {
		if (argp && argp[i]) continue;
		if (!argv[i]) {
			if (argp) argp[i] = true;
			continue;
		}
		std::stringcase arg (argv[i]);
		int oldnum = num;
		// macro file for each structure describing all fields 
		if (arg == "-mf" || arg == "/mf") {
			set_macrofile_type (macrofile_type::fields);
			++num;
		}
		// macro file for each structure describing the error messages
		else if (arg == "-me" || arg == "/me" ) {
			set_macrofile_type (macrofile_type::errors);
			++num;
		}
		// macro file for each structure for fields and errors (default)
		else if (arg == "-ma" || arg == "/ma" ) {
			set_macrofile_type (macrofile_type::all);
			++num;
		}
		// now set flag to indicated a processed option
		if (argp && (num > oldnum)) {
			argp[i] = true;
		}
	}
	return num;
}

/* Process a channel
   epics_macrofiles_processing::operator()
************************************************************************/
bool epics_macrofiles_processing::operator() (const ParseUtil::process_arg& arg)
{
	// Check if this is a valid
	if (arg.get_process_type() == process_type_enum::pt_invalid) {
		return false;
	}
	// ignore arrays
	if (!arg.is_atomic() &&
 		(arg.get_type_name().find ("ARRAY") != stringcase::npos)) {
		return false;
	}

	// make sure we have at least one record on the processing stack
	if (procstack.empty()) {
		procstack.push (macro_record());
	}

	// Get info
	macro_info minfo;
	minfo.ptype = arg.get_process_type();
	minfo.name = to_epics (arg.get_alias()).c_str();
	minfo.type_n = arg.get_type_name();
	minfo.readonly = arg.get_opc().is_readonly();

	// check if we need to process the top
	while ((procstack.size() > 1) && 
		   (minfo.name.compare (0, procstack.top().record.name.length(), 
		                        procstack.top().record.name) != 0)) {
		process_record (procstack.top(), static_cast<int>(procstack.size()) - 1);
		procstack.pop();
	}

	// Add the new field
	if (arg.get_opc().is_published() || !arg.is_atomic()) {
		procstack.top().fields.push_back(minfo);
	}
	// Check if this is an error structure
	int pos = static_cast<int>(minfo.type_n.length() - errorstruct.length());
	bool iserror = (minfo.type_n == errorstruct) ||
		((pos > 0) && (minfo.type_n[(stringcase::size_type)pos-1] == '.') && 
		 (minfo.type_n.compare (pos, std::stringcase::npos, errorstruct) == 0));
	if (iserror) {
		procstack.top().haserror = true;
		procstack.top().erroridx = static_cast<int>(procstack.top().fields.size()-1);
	}

	// check, if this is a structure
	if (!arg.is_atomic()) {
		// found a record: add to processing stack
		macro_record mrec;
		mrec.record = minfo;
		mrec.back = procstack.top().record;
		if (iserror) {
			mrec.iserror = true;
		}
		procstack.push (mrec);
		if (!mrec.iserror) {
			rec_num += 1;
		}
		return true;
	}
	return true;
}

/** Translate epics name to filename
************************************************************************/
std::stringcase epics_macrofiles_processing::to_filename (
	const std::stringcase& epicsname)
{
	std::stringcase ret(epicsname);
	std::stringcase::size_type pos;
	if (isTwinCAT3) {
		if ((pos = ret.find ('.')) == 0) ret.erase (0, 1);
		bool top = true;
		if ((pos = ret.find(':')) != stringcase::npos) {
			top = false;
			ret.erase(0, pos + 1);
		}
		if (!get_plcname().empty()) {
			if (ret.empty()) {
				// nothing
			}
			else if (top) {
				ret = "Top";
			}
			else {
				//ret.insert (0, get_plcname() + '_');
			}
		}
		while ((pos = ret.find_first_of ("-:.")) != stringcase::npos) ret[pos] = '_';
	}
	else {
		while ((pos = ret.find (':')) != stringcase::npos) ret.erase (pos, 1);
		if (!get_plcname().empty()) {
			if ((pos = ret.find ('-')) != stringcase::npos) {
				ret.insert (pos + 1, get_plcname() + '_');
			}
			else if ((pos = ret.find ('.')) != stringcase::npos) {
				ret.insert (pos, stringcase("_") + get_plcname());
			}
			else if (!ret.empty()) {
				(ret += '_') += get_plcname(); // yah!
			}
		}	
		while ((pos = ret.find ('-')) != stringcase::npos) ret[pos] = '_';
	}
	return ret;
}

/* Constants
   epics_macrofiles_processing::errorstruct
   epics_macrofiles_processing::errorlistext
************************************************************************/
const std::stringcase epics_macrofiles_processing::errorstruct = "ErrorStruct";
const std::stringcase epics_macrofiles_processing::errorlistext2 = "_Errors.exp";
const std::regex epics_macrofiles_processing::errormatchregex2 (
	"[^:]*:\\s*ErrorMessagesArray\\s*:=\\s*([^;]*);[^;]*", 
	std::regex_constants::icase);
const std::regex epics_macrofiles_processing::errorsearchregex ( 
	"'((\\$[\\$'LlNnPpRrTt\\d])|[^'\\$])*'", std::regex_constants::icase);
const std::stringcase epics_macrofiles_processing::errorlistext31 = "_Errors.TcGVL";
const std::regex epics_macrofiles_processing::errormatchregex31 (
	"[^:]*:\\s*ErrorMessagesArray\\s*:=\\s*\\[\\s*([^\\]]*)\\]\\s*;[^;]*", 
	std::regex_constants::icase);

/* Process a record
   epics_macrofiles_processing::process_record()
************************************************************************/
bool epics_macrofiles_processing::process_record (const macro_record& mrec, 
												  int level)
{
	// Check if we have a valid record
	if (mrec.record.ptype == process_type_enum::pt_invalid) {
		return false;
	}
	// No processing for erorr records
	if (mrec.iserror) {
		return true;
	}

	// Check if we need to read a _Errors.exp file containing a list of 
	// error messages
	bool havelist = false;
	std::vector<std::stringcase> errlist;
	if (mrec.haserror && 
		((get_macrofile_type() == macrofile_type::all) ||
		(get_macrofile_type() == macrofile_type::errors))) {
		bool succ = false;
		std::stringcase fname;
		if (isTwinCAT3) {
			fname = "";
			std::stringcase::size_type pos = mrec.record.type_n.find('.');
			if (pos == std::stringcase::npos) {
				fname += mrec.record.type_n + errorlistext31;
			}
			else {
				fname += mrec.record.type_n.substr(pos+1) + errorlistext31;
			}
			succ = open (fname, "r", true);
			// check if we have a field name
			if (!succ && !get_plcname().empty() &&
				mrec.record.name.rfind (get_plcname()) == 
				mrec.record.name.length() - get_plcname().length()) {
				std::stringcase::size_type pos = fname.rfind ("Struct");
				if (pos != stringcase::npos) {
					fname.insert (pos, get_plcname(), 0, 1);
				}
				succ = open (fname, "r", true);
			}
		}
		else {
			fname = mrec.record.type_n + errorlistext2;
			succ = open (fname, "r", true);
			// check if we have a field name
			if (!succ && !get_plcname().empty() &&
				mrec.record.name.rfind (get_plcname()) == 
					mrec.record.name.length() - get_plcname().length()) {
				std::stringcase::size_type pos = fname.rfind ("Struct");
				if (pos != stringcase::npos) {
					fname.insert (pos, get_plcname(), 0, 1);
				}
				succ = open (fname, "r", true);
			}
		}
		if (!succ) {
			if (missing.find (fname) == missing.end()) {
				missing.insert (fname);
				fprintf (stderr, "Cannot open %s\n", fname.c_str());
			}
		}
		else {
			havelist = true;
			// read file with list of error messages
			FILE* fp = get_file();
			fseek (fp, 0L, SEEK_END);
			size_t sz = ftell (fp);
			fseek (fp, 0L, SEEK_SET);
			if (sz > 1000000) sz = 1000000; // let's not get too crazy
			unsigned char* buf = new unsigned char [sz+1];
			sz = fread (buf, sizeof (char), sz, fp);
			buf[sz] = 0;
			for (size_t i = 0; i < sz; ++i) {
				if (isspace (buf[i])) buf[i] = ' '; // get rid of LF/CR
			}
			// check if it is formatted correctly
			std::cmatch match;
			if (std::regex_match ((const char*)buf, (const char*)buf+sz, match, 
				isTwinCAT3 ? errormatchregex31 : errormatchregex2)) {
				for (auto i = ++match.begin(); i != match.end(); ++i) {
					std::string found = i->str();
					// search and iterate over single quote strings
					std::regex_iterator<std::string::iterator> rit 
						(found.begin(), found.end(), errorsearchregex);
					std::regex_iterator<std::string::iterator> rend;
					while (rit != rend) {
						// found one
						std::stringcase msg = rit->str().c_str();
						// trim single quotes
						msg.erase (0, 1);
						msg.erase (msg.length()-1, 1);
						// trim control characters
						msg = std::regex_replace (msg, std::regex ("\\$([LlNnPpRr]|\\d\\d?)"), "");
						// unescape $' and $$
						msg = std::regex_replace (msg, std::regex ("\\$([\\$'])"), "$1");
						// unesacpe $t
						msg = std::regex_replace (msg, std::regex ("\\$([tT])"), " ");
						//printf ("found an error message `%s`\n", msg.c_str());
						errlist.push_back (msg);
						++rit;
					}
				}
			}
			delete [] buf;
			close();
		}
	}

	// open output file
	if (!open (to_filename (mrec.record.name) + ".aml", "w")) {
		fprintf (stderr, "Failed to process %s.\n", mrec.record.name.c_str());
		return false;
	}

	// write output file
	FILE* fp = get_file();
	if (get_plcname().empty()) {
		fprintf (fp, "PLC=Unknown\n");
	}
	else {
		fprintf (fp, "PLC=%s,\n", get_plcname().c_str());
	}
	fprintf (fp, "CHN=%s,\n", mrec.record.name.c_str());

	// get ifo
	auto colon = mrec.record.name.find (':');
	auto dash = mrec.record.name.find ('-');
	std::stringcase ifo;
	if (colon != stringcase::npos) {
		ifo = mrec.record.name.substr (0, colon);
	}
	else {
		ifo = mrec.record.name;
	}
	// get sys and sub
	std::stringcase sys;
	std::stringcase sub;
	if (dash != stringcase::npos) {
		if (colon == stringcase::npos) {
			sys = mrec.record.name.substr (0, dash);
			sub = mrec.record.name.substr (dash + 1, stringcase::npos);
		} 
		else if (dash > colon) {
			sys = mrec.record.name.substr (colon + 1, dash - colon - 1);
			sub = mrec.record.name.substr (dash + 1, stringcase::npos);
		}
		else {
			sys = "";
			sub = mrec.record.name;
		}
	}
	else {
		sub = "";
		if (colon != stringcase::npos) {
			sys = mrec.record.name.substr (colon + 1, stringcase::npos);
		}
		else {
			sys = "";
		}
	}
	fprintf (fp, "IFO=%s,\n", ifo.c_str());
	std::stringcase lifo = ifo;
	for (unsigned int i = 0; i < lifo.length(); ++i) lifo[i] = tolower (lifo[i]);
	fprintf (fp, "ifo=%s,\n", lifo.c_str());
	fprintf (fp, "SYS=%s,\n", sys.c_str());
	fprintf (fp, "SUB=%s,\n", sub.c_str());
	fprintf (fp, "LVL=%i,\n", level);

	// screen names
	fprintf (fp, "itself=%s,\n", to_filename (mrec.record.name).c_str());
	fprintf (fp, "related=%s,\n", to_filename (mrec.record.name).c_str());
	fprintf (fp, "back=%s,\n", to_filename (mrec.back.name).c_str());
	// write has errors
	fprintf (fp, "haserrors=%i,\n", mrec.haserror ? 1 : 0);
	if (mrec.haserror && (mrec.erroridx >= 0) && (mrec.erroridx < (int)mrec.fields.size())) {
		fprintf (fp, "errfld=%s,\n", mrec.fields[mrec.erroridx].name.c_str());
	}
	else {
		fprintf (fp, "errfld=,\n");
	}

	// Error messages
	if ((get_macrofile_type() == macrofile_type::all) || 
		(get_macrofile_type() == macrofile_type::errors)) {
		int num = 0;
		for (auto i = errlist.begin(); (i != errlist.end()) && (num < 32); ++i, ++num) {
			// write error message
			fprintf (fp, "err%i=\"%s\",\n", num, i->c_str());
			// check if we have a field name
			if (!get_plcname().empty() &&
				mrec.record.name.rfind (get_plcname()) == 
				mrec.record.name.length() - get_plcname().length()) {
				// just prepend IFO if all caps
				std::stringcase suberr = to_filename (ifo + ":" + *i);
				std::stringcase::size_type pos;
				if ((pos = suberr.find ('-')) != stringcase::npos) suberr[pos] = '_';
				bool issuberr = (suberr.length() > 0) && (isalpha (suberr[0]));
				for (const auto& j : suberr) {
					if (!isalnum(j) && (j != '_')) {
						issuberr = false;
						break;
					}
				}
				if (issuberr) {
					fprintf (fp, "nxt%i=%s,\n", num, suberr.c_str());
				}
			}
			else {
				// search among sub fields only
				const macro_info* pinfo = nullptr;
				//std::stringcase suberr = mrec.record.name + "_" + *i;
				auto mlen =  mrec.record.name.length();
				for (const auto& j : mrec.fields) {
					if ((j.ptype == process_type_enum::pt_binary) &&
						(mlen + i->length() + 1 == j.name.length()) &&
						!isalnum (j.name[mlen]) &&
						(j.name.compare (mlen + 1, std::stringcase::npos, *i) == 0)) {
						pinfo = &j;
						break;
					}
					//if ((j.name == suberr) && (j.ptype == pt_binary)) {
					//	pinfo = &j;
					//	break;
					//}
				}
				if (pinfo) {
					fprintf (fp, "nxt%i=%s,\n", num, to_filename (pinfo->name).c_str());
				}
			}
		}
		// write list length
		fprintf (fp, "errors=%i,\n", num);
	}

	// Fields
	if ((get_macrofile_type() == macrofile_type::all) || 
		(get_macrofile_type() == macrofile_type::fields)) {
		int num = 0;
		for (unsigned int idx = 0; idx < mrec.fields.size(); ++idx) {
			const auto& i = mrec.fields[idx]; 
			// skip error struct
			if (mrec.erroridx == idx) continue;
			// set field type
			switch (i.ptype) {
			case process_type_enum::pt_bool:
				fprintf (fp, "fio%i=%s,\n", num, i.readonly ? "bi" : "bo");
				break;
			case process_type_enum::pt_enum:
				fprintf (fp, "fio%i=%s,\n", num, i.readonly ? "mbbi" : "mbbo");
				break;
			case process_type_enum::pt_int:
				fprintf (fp, "fio%i=%s,\n", num, i.readonly ? "longin" : "longout");
				break;
			case process_type_enum::pt_real:
				fprintf (fp, "fio%i=%s,\n", num, i.readonly ? "ai" : "ao");
				break;
			case process_type_enum::pt_string:
				fprintf (fp, "fio%i=%s,\n", num, i.readonly ? "stringin" : "stringout");
				break;
			case process_type_enum::pt_binary:
				fprintf (fp, "fio%i=%s,\n", num, "link");
				break;
			case process_type_enum::pt_invalid:
			default:
				continue;
			}
			// set field name
			if (i.ptype == process_type_enum::pt_binary) {
				fprintf (fp, "fld%i=%s,\n", num, to_filename (i.name).c_str());
			}
			else {
				fprintf (fp, "fld%i=%s,\n", num, i.name.c_str());
			}
			++num;
		}
		fprintf (fp, "fields=%i,\n", num);
	}

	close();
	return true;
}


/* Option processing
   epics_db_processing::epics_db_processing
************************************************************************/
epics_db_processing::epics_db_processing (
		const std::stringcase& fname,
		int argc, const char* const argv[], bool argp[])
	: epics_conversion (argc, argv, argp), 
	  split_io_support (fname, argc, argv, argp),
	  device_support (device_support_type::tc_name),
	  string_support(string_support_type::vary_string),
	  int_support(int_support_type::int_auto)
{
	mygetopt (argc, argv, argp); 
}

/* Option processing
   epics_db_processing::getopt
************************************************************************/
int epics_db_processing::getopt (int argc, const char* const argv[], 
								 bool argp[]) 
{
	return epics_conversion::getopt (argc, argv, argp) +
		   mygetopt (argc, argv, argp); 
}

/* Option processing
   epics_db_processing::mygetopt
************************************************************************/
int epics_db_processing::mygetopt (int argc, const char* const argv[], 
								   bool argp[]) 
{
	int num = 0;
	for (int i = 1; i < argc; ++i) {
		if (argp && argp[i]) continue;
		if (!argv[i]) {
			if (argp) argp[i] = true;
			continue;
		}
		std::stringcase arg (argv[i]);
		int oldnum = num;
		// Uses OPC name in INPUT/OUTPUT field (default) 
		if (arg == "-devopc" || arg == "/devopc") {
			set_device_support (device_support_type::opc_name);
			++num;
		}
		// Uses TwinCAT name in INPUT/OUTPUT fields instead of OPC
		else if (arg == "-devtc" || arg == "/devtc" ) {
			set_device_support (device_support_type::tc_name);
			++num;
		}
		else if (arg == "-ss" || arg == "/ss") {
			set_string_support(string_support_type::short_string);
			++num;
		}
		else if (arg == "-sl" || arg == "/sl") {
			set_string_support(string_support_type::long_string);
			++num;
		}
		else if (arg == "-sd" || arg == "/sd") {
			set_string_support(string_support_type::vary_string);
			++num;
		}
		else if (arg == "-is" || arg == "/is") {
			set_int_support(int_support_type::int_32);
			++num;
		}
		else if (arg == "-il" || arg == "/il") {
			set_int_support(int_support_type::int_64);
			++num;
		}
		else if (arg == "-id" || arg == "/id") {
			set_int_support(int_support_type::int_auto);
			++num;
		}
		// now set flag to indicated a processed option
		if (argp && (num > oldnum)) {
			argp[i] = true;
		}
	}
	return num;
}

/* Process a channel
   epics_db_processing::operator()
************************************************************************/
bool epics_db_processing::operator() (const process_arg& arg)
{
	// quit if not atomic
	if (!arg.is_atomic()) {
		return false;
	}

	// published?
	if (!arg.get_opc().is_published()) {
		return false;
	}

	// readonly?
	bool readonly = arg.get_opc().is_readonly();
	increment (readonly);

	// default process type conversion
	stringcase tname;
	switch (arg.get_process_type()) {
	case process_type_enum::pt_int:
		if (((arg.get_size() == 8) &&
			(get_int_support() == int_support_type::int_auto)) ||
			(get_int_support() == int_support_type::int_64))	{
			tname = readonly ? "int64in" : "int64out";
		}
		else {
			tname = readonly ? "longin" : "longout";
		}
		break;
	case process_type_enum::pt_real:
		tname = readonly ? "ai" : "ao";
		break;
	case process_type_enum::pt_bool:
		tname = readonly ? "bi" : "bo";
		break;
	case process_type_enum::pt_string:
		if (((arg.get_size() >= MAX_EPICS_STRING) && 
			 (get_string_support() == string_support_type::vary_string)) ||
			(get_string_support() == string_support_type::long_string))	{
			tname = readonly ? "lsi" : "lso";
		}
		else {
			tname = readonly ? "stringin" : "stringout";
		}
		break;
	case process_type_enum::pt_enum:
		tname = readonly ? "mbbi" : "mbbo";
		break;
	default:
		fprintf (stderr, "Unknown type %s for %s\n", 
			arg.get_type_name().c_str(), arg.get_name().c_str());
		return false;
	}
	// check OPC_PROP_RECTYPE
	stringcase s;
	if (arg.get_opc().get_property (OPC_PROP_RECTYPE, s)) {
		tname = s;
	}

	// check record header
	string epicsname = to_epics (arg.get_alias());
	if (epicsname.size() > MAX_EPICS_CHANNEL) {
		fprintf (stderr, "Warning: channel name %s too long by %i\n", 
			epicsname.c_str(), static_cast<int>(epicsname.size() - MAX_EPICS_CHANNEL));
		return false;
	}
	// now print header
	fprintf(get_file(), "record(%s,\"%s\") {\n", tname.c_str(), epicsname.c_str());

	// string size for lsi/lso
	if ((tname == "lsi") || (tname == "lso")) {
		int len = arg.get_size() + 1;
		if (len > MAX_EPICS_LONGSTRING) len = MAX_EPICS_LONGSTRING;
		process_field_numeric(EPICS_DB_SIZV, len);
	}

	// check OPC_PROP_DESC
	if (arg.get_opc().get_property (OPC_PROP_DESC, s)) {
		if (s.size() > MAX_EPICS_DESC) {
			fprintf (stderr, "Warning: DESC for %s too long by %i\n", 
				arg.get_name().c_str(), static_cast<int>(s.size() - MAX_EPICS_DESC));
		}
		process_field_string (EPICS_DB_DESC, s);
	}
	// add SCAN
	process_field_string (EPICS_DB_SCAN, readonly ? "I/O Intr" : "Passive");
	// check for DTYP
	stringcase dtyp = device_support == device_support_type::tc_name ? "tcat" : "opc";
	if (arg.get_opc().get_property (OPC_PROP_DTYP, s)) {
		if (s.find ("raw") == stringcase::npos) {
			dtyp = device_support == device_support_type::tc_name ? "tcat raw" : "opcRaw";
		}
	}
	process_field_string (EPICS_DB_DTYP, dtyp);
	// INPUT/OUTPUT field
	stringcase inplink;
	stringcase servername;
	switch (device_support) {
	case device_support_type::opc_name:
	default:
		// check for server
		servername = "opc";
		arg.get_opc().get_property (OPC_PROP_SERVER, servername);
		// input/output link
		inplink = stringcase ("@") + servername + arg.get_name(); 
		break;
	case device_support_type::tc_name:
		// input/output link
		inplink = stringcase ("@") + arg.get_full();
		break;
	}
	process_field_string (readonly ? EPICS_DB_INP : EPICS_DB_OUT, inplink);

	// check for TSE
	int tse = -2;
	arg.get_opc().get_property (OPC_PROP_TSE, tse);
	process_field_numeric (EPICS_DB_TSE, tse);
	// check for PINI
	//int pini = readonly ? 1 : 0;
	int pini = 0;
	arg.get_opc().get_property (OPC_PROP_PINI, tse);
	process_field_numeric (EPICS_DB_PINI, pini);

	// go through properties
	for (const auto& f : arg.get_opc().get_properties()) 
	{
		switch (f.first) {
		case OPC_PROP_UNIT :
			process_field_string (EPICS_DB_EGU, f.second);
			break;
		case OPC_PROP_DESC :
			// processed above
			break;
		case OPC_PROP_HIEU :
			process_field_numeric (EPICS_DB_HOPR, f.second);
			break;
		case OPC_PROP_LOEU :
			process_field_numeric (EPICS_DB_LOPR, f.second);
			break;
		case OPC_PROP_HIRANGE :
			process_field_numeric (EPICS_DB_DRVH, f.second);
			break;
		case OPC_PROP_LORANGE :
			process_field_numeric (EPICS_DB_DRVL, f.second);
			break;
		case OPC_PROP_CLOSE :
			process_field_string (EPICS_DB_ONAM, f.second);
			break;
		case OPC_PROP_OPEN :
			process_field_string (EPICS_DB_ZNAM, f.second);
			break;
		case OPC_PROP_PREC :
			process_field_numeric (EPICS_DB_PREC, f.second);
			break;
		case OPC_PROP_ZRST :
		case OPC_PROP_ZRST + 1 :
		case OPC_PROP_ZRST + 2 :
		case OPC_PROP_ZRST + 3 :
		case OPC_PROP_ZRST + 4 :
		case OPC_PROP_ZRST + 5 :
		case OPC_PROP_ZRST + 6 :
		case OPC_PROP_ZRST + 7 :
		case OPC_PROP_ZRST + 8 :
		case OPC_PROP_ZRST + 9 :
		case OPC_PROP_ZRST + 10 :
		case OPC_PROP_ZRST + 11 :
		case OPC_PROP_ZRST + 12 :
		case OPC_PROP_ZRST + 13 :
		case OPC_PROP_ZRST + 14 :
		case OPC_PROP_FFST :
			if ((tname == "mbbi") || (tname == "mbbo")) {
				process_field_numeric (EPICS_DB_ZRVL[f.first-OPC_PROP_ZRST], f.first-OPC_PROP_ZRST);
				process_field_string (EPICS_DB_ZRST[f.first-OPC_PROP_ZRST], f.second);
			}
			break;
		case OPC_PROP_RECTYPE :
		case OPC_PROP_INOUT :
		case OPC_PROP_TSE :
		case OPC_PROP_PINI :
		case OPC_PROP_DTYP :
		case OPC_PROP_SERVER :
		case OPC_PROP_PLCNAME :
		case OPC_PROP_ALIAS :
			// processed above
			break;
			// alarm
		case OPC_PROP_ALMOSV:
			if ((tname == "bi") || (tname == "bo")) {
				process_field_alarm (EPICS_DB_OSV, f.second);
			}
			break;
		case OPC_PROP_ALMZSV:
			if ((tname == "bi") || (tname == "bo")) {
				process_field_alarm (EPICS_DB_ZSV, f.second);
			}
			break;
		case OPC_PROP_ALMCOSV:
			if ((tname == "bi") || (tname == "bo") || (tname == "mbbi") || (tname == "mbbo")) {
				process_field_alarm (EPICS_DB_COSV, f.second);
			}
			break;
		case OPC_PROP_ALMUNSV:
			if ((tname == "mbbi") || (tname == "mbbo")) {
				process_field_alarm (EPICS_DB_UNSV, f.second);
			}
			break;
		case OPC_PROP_ALMZRSV:
		case OPC_PROP_ALMZRSV + 1:
		case OPC_PROP_ALMZRSV + 2:
		case OPC_PROP_ALMZRSV + 3:
		case OPC_PROP_ALMZRSV + 4:
		case OPC_PROP_ALMZRSV + 5:
		case OPC_PROP_ALMZRSV + 6:
		case OPC_PROP_ALMZRSV + 7:
		case OPC_PROP_ALMZRSV + 8:
		case OPC_PROP_ALMZRSV + 9:
		case OPC_PROP_ALMZRSV + 10:
		case OPC_PROP_ALMZRSV + 11:
		case OPC_PROP_ALMZRSV + 12:
		case OPC_PROP_ALMZRSV + 13:
		case OPC_PROP_ALMZRSV + 14:
		case OPC_PROP_ALMFFSV:
			if ((tname == "mbbi") || (tname == "mbbo")) {
				process_field_alarm (EPICS_DB_ZRSV[f.first-OPC_PROP_ALMZRSV], f.second);
			}
			break;
		case OPC_PROP_ALMHH:
			if ((tname == "ai") || (tname == "ao") || (tname == "longin") || (tname == "longout") || (tname == "int64in") || (tname == "int64out")) {
				process_field_numeric (EPICS_DB_HIHI, f.second);
				std::stringcase alarmsv;
				if (!arg.get_opc().get_property (OPC_PROP_ALMHHSV, alarmsv)) {
					process_field_alarm (EPICS_DB_HHSV, EPICS_DB_MAJOR);
				}
			}
			break;
		case OPC_PROP_ALMH:
			if ((tname == "ai") || (tname == "ao") || (tname == "longin") || (tname == "longout") || (tname == "int64in") || (tname == "int64out")) {
				process_field_numeric (EPICS_DB_HIGH, f.second);
				std::stringcase alarmsv;
				if (!arg.get_opc().get_property (OPC_PROP_ALMHSV, alarmsv)) {
					process_field_alarm (EPICS_DB_HSV, EPICS_DB_MINOR);
				}
			}
			break;
		case OPC_PROP_ALML:
			if ((tname == "ai") || (tname == "ao") || (tname == "longin") || (tname == "longout") || (tname == "int64in") || (tname == "int64out")) {
				process_field_numeric (EPICS_DB_LOW, f.second);
				std::stringcase alarmsv;
				if (!arg.get_opc().get_property (OPC_PROP_ALMLSV, alarmsv)) {
					process_field_alarm (EPICS_DB_LSV, EPICS_DB_MINOR);
				}
			}
			break;
		case OPC_PROP_ALMLL:
			if ((tname == "ai") || (tname == "ao") || (tname == "longin") || (tname == "longout") || (tname == "int64in") || (tname == "int64out")) {
				process_field_numeric (EPICS_DB_LOLO, f.second);
				std::stringcase alarmsv;
				if (!arg.get_opc().get_property (OPC_PROP_ALMLLSV, alarmsv)) {
					process_field_alarm (EPICS_DB_LLSV, EPICS_DB_MAJOR);
				}
			}
			break;
		case OPC_PROP_ALMHHSV:
			if ((tname == "ai") || (tname == "ao") || (tname == "longin") || (tname == "longout") || (tname == "int64in") || (tname == "int64out")) {
				process_field_alarm (EPICS_DB_HHSV, f.second);
			}
			break;
		case OPC_PROP_ALMHSV:
			if ((tname == "ai") || (tname == "ao") || (tname == "longin") || (tname == "longout") || (tname == "int64in") || (tname == "int64out")) {
				process_field_alarm (EPICS_DB_HSV, f.second);
			}
			break;
		case OPC_PROP_ALMLSV:
			if ((tname == "ai") || (tname == "ao") || (tname == "longin") || (tname == "longout") || (tname == "int64in") || (tname == "int64out")) {
				process_field_alarm (EPICS_DB_LSV, f.second);
			}
			break;
		case OPC_PROP_ALMLLSV:
			if ((tname == "ai") || (tname == "ao") || (tname == "longin") || (tname == "longout") || (tname == "int64in") || (tname == "int64out")) {
				process_field_alarm (EPICS_DB_LLSV, f.second);
			}
			break;
		case OPC_PROP_ALMDB:
			if ((tname == "ai") || (tname == "ao") || (tname == "longin") || (tname == "longout") || (tname == "int64in") || (tname == "int64out")) {
				process_field_numeric (EPICS_DB_HYST, f.second);
			}
			break;
			// unknown
		default:
			// Process unchecked field,value pairs
			if ((f.first >= OPC_PROP_FIELD_BEG) && (f.first < OPC_PROP_FIELD_END)) {
				std::stringcase::size_type pos = f.second.find(',');
				if (pos != std::stringcase::npos) {
					std::stringcase field = f.second.substr (0, pos);
					trim_space (field);
					std::stringcase value = f.second.substr (pos + 1);
					trim_space (value);
					process_field_string (field, value);
				}
				else {
					fprintf(stderr, "Property %s is not a field,value pair\n", arg.get_name().c_str());
				}
			}
			// Error: Unknown property
			else if (f.first >= 1000) {
				fprintf (stderr, "Unknown property %i for %s\n", f.first, arg.get_name().c_str());
			}
			break;
		}
	}
	// end with closing bracket
	fprintf(get_file(), "}\n");
	return true;
}

/* Process a field
epics_db_processing::process_field_string
************************************************************************/
bool epics_db_processing::process_field_string (stringcase name, 
												stringcase val)
{
	fprintf (get_file(), "\tfield(%s,\"%s\")\n", name.c_str(), val.c_str());
	return true;
}

/* Process a field
epics_db_processing::process_field_numeric
************************************************************************/
bool epics_db_processing::process_field_numeric (stringcase name, int val)
{
	char buf[40];
	sprintf_s (buf, sizeof(buf), "%i", val);
	return process_field_string (name, buf);
}

/* Process a field
epics_db_processing::process_field_numeric
************************************************************************/
bool epics_db_processing::process_field_numeric (stringcase name, double val)
{
	char buf[40];
	sprintf_s (buf, sizeof(buf), "%g", val);
	return process_field_string (name, buf);
}

/* Process a field
epics_db_processing::process_field_numeric
************************************************************************/
bool epics_db_processing::process_field_numeric (stringcase name, 
												 stringcase val)
{
	if (val.find_first_of (".Ee") != stringcase::npos) {
		double v = strtod (val.c_str(), NULL);
		return process_field_numeric (name, v);
	}
	else {
		int v = strtol (val.c_str(), NULL, 10);
		return process_field_numeric (name, v);
	}
}

/* Process a field
epics_db_processing::process_field_alarm
************************************************************************/
bool epics_db_processing::process_field_alarm (stringcase name, 
											   stringcase severity)
{
	if ((severity == EPICS_DB_NOALARM) || (severity == EPICS_DB_MINOR) || 
		(severity == EPICS_DB_MAJOR)) {
		fprintf (get_file(), "\tfield(%s,\"%s\")\n", name.c_str(), severity.c_str());
		return true;
	}
	else {
		fprintf (stderr, "Unknown alarm severity %s for %s\n", severity.c_str(), name.c_str());
		return false;
	}
}


}
