#pragma once
#include "stdafx.h"
#include "ParseTpy.h"

/** @file TpyToEpics.h
	Header which includes classes to convert a parsed TwinCAT tpy into
	an EPICS database. 
	It includes "ParseTpy.h"
 ************************************************************************/

/** @namespace EpicsTpy
	EpicsTpy name space 
	@brief Namespace for tpy-db conversion
 ************************************************************************/
namespace EpicsTpy {

/** @defgroup epicstpyutil Utility functions and classes
 ************************************************************************/
/** @{ */

/** This enum describes the TwinCAT/opc to EPICS conversion rule
 ************************************************************************/
enum tc_epics_conv {
	/// No conversion
	no_conversion, 
	/// Convert '.' to '_'
	no_dot, 
	/// LIGO standard conversion:
	/// Eliminate leading '.', 
	/// Replace sceond '.' with ':',
	/// Replace third '.' with '-',
	/// Replace all other '.' with '_'
	ligo_std,
	/// LIGO standard conversion for vacuum channels:
	/// Eliminate leading '.', 
	/// Replace sceond '.' with '-',
	/// Replace third '.' with ':',
	/// Replace all other '.' with '_'
	ligo_vac
};

/** This enum describes the case conversion rule
 ************************************************************************/
enum case_type {
	/// Preserve the case
	preserve_case, 
	/// Convert to upper case
	upper_case, 
	/// Convert to lower case
	lower_case
};

/** Epics channel conversion arguments
Epics channels are generated from opc through a conversion rule
************************************************************************/
class epics_conversion {
public:
	/// Default constructor
	epics_conversion()
		: conv_rule (ligo_std), case_epics_names (upper_case), 
		no_leading_dot (true), no_array_index (true) {}
	/// Constructor
	/// @param caseconv Case conversion specification
	/// @param noindex Eliminate array indices '[n]' with '_n'
	epics_conversion (case_type caseconv, bool noindex)
		: conv_rule (ligo_std), case_epics_names (caseconv), 
		no_leading_dot (true), no_array_index (noindex) {}
	/// Constructor
	/// @param epics_conv Epics conversion rule
	/// @param caseconv Case conversion specification
	/// @param noldot Eliminate leading dot in a name
	/// @param noindex Eliminate array indices '[n]' with '_n'
	epics_conversion (tc_epics_conv epics_conv, case_type caseconv, 
		bool noldot, bool noindex)
		: conv_rule (epics_conv), case_epics_names (caseconv), 
		no_leading_dot (noldot), no_array_index (noindex) {}
	/// Constructor
	/// Command line arguments will override default parameters when specified
	/// The format is the same as the arguments passed to the main program
	/// argv[0] is program name and will be ignored
	/// @param argc Number of command line arguments
	/// @param argv List of command line arguments, same format as in main()
	/// @param argp Excluded/processed arguments (in/out), array length must be argc
	epics_conversion (int argc, const char* const argv[], bool argp[] = 0)
		: conv_rule (ligo_std), case_epics_names (upper_case), 
		no_leading_dot (true), no_array_index (true) { 
		getopt (argc, argv ,argp);}

	/// Parse a command line
	/// The format is the same as the arguments passed to the main program
	/// argv[0] is program name and will be ignored
	/// The argp boolean array can be used to pass in a list of already
	/// processed (and to be ignored) command line arguments. This list,
	/// if supplied, must be at least argc long. Upon return, newly processed
	/// arguments are also marked as processed in this list. The arguments are:
	///
	/// /rn: Does not apply any special conversion rules
	/// /rd: Replaces dots with underscores in channel names
	/// /rl: LIGO standard conversion rule (default)
	/// /cp: Preserve case in EPICS channel names
	/// /cu: Force upper case in EPICS channel names (default)
	/// /cl: Force lower case in EPICS channel names
	/// /nd: Eliminates leading dot in channel name (default)
	/// /yd: Leaves leading dot in channel name
	/// /ni: Replaces array brackets with underscore (default)
	/// /yi: Leave array indices as is
	///
	/// Command line arguments can use '-' instead of a '/'. Capitalization does
	/// not matter. getopt will only override arguments that are specifically 
	/// specified. It relies on the contructors to provide the defaults.
	/// @param argc Number of command line arguments
	/// @param argv List of command line arguments, same format as in main()
	/// @param argp Excluded/processed arguments (in/out), array length must be argc
	/// @return Number of arguments processed
	int getopt (int argc, const char* const argv[], bool argp[] = 0);

	/// Get the conversion rule
	tc_epics_conv get_conversion_rule () const { return conv_rule; }
	/// Set the conversion rule
	void set_conversion_rule (tc_epics_conv epics_conv) {
		conv_rule = epics_conv; }
	/// Get the conversion rule
	case_type get_case_rule () const { return case_epics_names; }
	/// Set the conversion rule
	void set_case_rule (case_type epics_conv) {
		case_epics_names = epics_conv; }
	/// Get the leadin dot rule
	bool get_dot_rule () const { return no_leading_dot; }
	/// Set the leading dot rule
	void set_dot_rule (bool noldot) {
		no_leading_dot = noldot; }
	/// Get the array index rule
	bool get_array_rule () const { return no_array_index; }
	/// Set the array conversion rule
	void set_array_rule (bool noindex) {
		no_array_index = noindex; }

	/// Converts a TwinCAT or OPC name to an EPICS channel name
	/// @param name TwinCAT/opc name
	/// @return EPICS name
	std::string to_epics (const std::stringcase& name) const;

protected:
	/// Conversion rule
	tc_epics_conv	conv_rule;
	/// Case conversion rule
	case_type		case_epics_names;
	/// Leading dot conversion rule
	bool			no_leading_dot;
	/// Array index conversion rule
	bool			no_array_index;
};

/** Split file IO support
Output can be split in multiple files if the number of channels
exceeds the maximum specified for a file
************************************************************************/
class split_io_support {
public:
	/// Default constructor
	split_io_support () 
		: error (true), split_io (false), split_n (0), 
		outf(stdout), outf_in (nullptr), outf_io (nullptr), 
		rec_num (0), rec_num_in (0), rec_num_io (0), 
		file_num_in (1), file_num_io (1) {}

	/// Constructor
	explicit split_io_support (const std::stringcase& fname, 
		bool split = false, int max = 0) 
		: error (false), split_io (split), split_n (max), 
		outf(stdout), outf_in (nullptr), outf_io (nullptr), 
		rec_num (0), rec_num_in (0), rec_num_io (0), 
		file_num_in (1), file_num_io (1) 
	{ set_filename (fname); }
	/// Constructor
	/// Command line arguments will override default parameters when specified
	/// The format is the same as the arguments passed to the main program
	/// argv[0] is program name and will be ignored
	/// @param argc Number of command line arguments
	/// @param argv List of command line arguments, same format as in main()
	/// @param argp Excluded/processed arguments (in/out), array length must be argc
	split_io_support (const std::stringcase& fname,
		int argc, const char* const argv[], bool argp[] = 0) 
		: error (false), split_io (false), split_n (0), 
		outf(stdout), outf_in (nullptr), outf_io (nullptr), 
		rec_num (0), rec_num_in (0), rec_num_io (0), 
		file_num_in (1), file_num_io (1) { 
		getopt (argc, argv, argp); set_filename (fname); }
	/// Destructor
	~split_io_support ();
	/// Copy constructor
	/// File pointers will be moved over and the original ones will become invalid.
	split_io_support (const split_io_support&);
	/// Assignment
	/// File pointers will be moved over andt the original ones will become invalid.
	split_io_support& operator= (const split_io_support&);

	/// Return error
	bool operator! () const { return error; }

	/// Increase the channel number
	/// @param readonly Inidcates if channel is readonly
	/// @return True if no error
	bool increment (bool readonly);
	/// Flush contents of output files
	void flush();

	/// Get output file
	FILE* get_file () const {return outf; }
	/// Get output filename
	const std::stringcase& get_filename () const 
	{ return outfilename; }
	/// Is output split?
	bool is_split() const { return split_io; }
	/// Maximum of channels per file
	int get_max() const { return split_n; }

	/// Get number of processed channels
	int get_processed_total() const { return rec_num; }
	/// Get number of processed readonly channels
	int get_processed_readonly() const { return rec_num_in; }
	/// Get number of processed input/ouput channels
	int get_processed_io() const { return rec_num_io; }

protected:
	/// Set output filename
	void set_filename (const std::stringcase& fname);
	/// Close files
	void close();
	/// set split
	void set_split (bool split) { split_io = split; }
	/// Set maximum of channels per file
	void set_max (int max) { split_n = (max <= 0) ? 0 : max; }

	/// Parse a command line
	/// The format is the same as the arguments passed to the main program
	/// argv[0] is program name and will be ignored
	/// The argp boolean array can be used to pass in a list of already
	/// processed (and to be ignored) command line arguments. This list,
	/// if supplied, must be at least argc long. Upon return, newly processed
	/// arguments are also marked as processed in this list. The arguments are:
	///
	/// /ysio: Splits database into input only and input/ouput recrods 
	/// /nsio: Does not split database by record type (default)
	/// /sn 'num': Splits database or listing into files with no more than num records
	/// /sn 0: Does not split database or listing into multiple files (default)
	///
	/// Command line arguments can use '-' instead of a '/'. Capitalization does
	/// not matter. getopt will only override arguments that are specifically 
	/// specified. It relies on the contructors to provide the defaults.
	/// @param argc Number of command line arguments
	/// @param argv List of command line arguments, same format as in main()
	/// @param argp Excluded/processed arguments (in/out), array length must be argc
	/// @return Number of arguments processed
	int getopt (int argc, const char* const argv[], bool argp[] = 0);

	/// Error 
	bool			error;
	/// Output filename
	std::stringcase		outfilename;
	/// Split output into read only channels and input/output channels
	bool			split_io;
	/// Maximum number of channels per file; 0 indicates no limit
	int				split_n;
	/// Output file
	mutable FILE*	outf;
	/// Output file for read only channels
	mutable FILE*	outf_in;
	/// Output file for input/output channels
	mutable FILE*	outf_io;

	/// Current number of processed channels (records)
	int				rec_num;
	/// Current number of processed read only channels (records)
	int				rec_num_in;
	/// Current number of processed input/output channels (records)
	int				rec_num_io;
	/// Current file number of processed read only channels (records)
	int				file_num_in;
	/// Current file number of processed input/output channels (records)
	int				file_num_io;

	/// Contains the readonly file number in string format
	std::stringcase		file_num_in_s;
	/// Contains the input/output file number in string format
	std::stringcase		file_num_io_s;
	/// Contains the file extenstion for readonly files
	std::stringcase		file_in_s;
	/// Contains the file extenstion for input/output files
	std::stringcase		file_io_s;
private:
};

/** @} */

/** @defgroup epicstpyprocessing Classes for converting a paresed tpy
    into an EPICS database
 ************************************************************************/
/** @{ */

/** This enum describes the type of listing to produce
 ************************************************************************/
enum listing_type {
	/// Standard listing using TwinCAT/OPC names
	listing_standard, 
	/// Autoburt listing
	listing_autoburt
};

/** Class for generatig a channel list
************************************************************************/
class epics_list_processing : 
	public epics_conversion, public split_io_support {
public:
	/// Default constructor
	epics_list_processing() 
		: listing (listing_standard), verbose (false) {}
	/// Constructor
	/// @param ltype Type of listing
	/// @param ll long listing
	explicit epics_list_processing (listing_type ltype, 
		bool ll = false) 
		: listing (listing_standard), verbose (ll) {}
	/// Constructor
	/// Command line arguments will override default parameters when specified
	/// The format is the same as the arguments passed to the main program
	/// argv[0] is program name and will be ignored
	/// Processed options with epics_conversion::getopt, 
	/// split_io_support::getopt and mygetopt().
	/// @param fname Output filename
	/// @param argc Number of command line arguments
	/// @param argv List of command line arguments, same format as in main()
	/// @param argp Excluded/processed arguments (in/out), array length must be argc
	epics_list_processing (const std::stringcase& fname,
		int argc, const char* const argv[], bool argp[] = 0);

	/// Parse a command line
	/// Processed options with epics_conversion::getopt and mygetopt().
	/// @param argc Number of command line arguments
	/// @param argv List of command line arguments, same format as in main()
	/// @param argp Excluded/processed arguments (in/out), array length must be argc
	/// @return Number of arguments processed
	int getopt (int argc, const char* const argv[], bool argp[] = 0);
	/// Parse a command line
	/// The format is the same as the arguments passed to the main program
	/// argv[0] is program name and will be ignored
	/// The argp boolean array can be used to pass in a list of already
	/// processed (and to be ignored) command line arguments. This list,
	/// if supplied, must be at least argc long. Upon return, newly processed
	/// arguments are also marked as processed in this list. The arguments are:
	///
	/// /l: Generates a standard listing (default)
	/// /ll: Generates a long listing
	/// /lb: Generates an autoburt save/restore file
	///
	/// Command line arguments can use '-' instead of a '/'. Capitalization does
	/// not matter. getopt will only override arguments that are specifically 
	/// specified. It relies on the contructors to provide the defaults.
	/// @param argc Number of command line arguments
	/// @param argv List of command line arguments, same format as in main()
	/// @param argp Excluded/processed arguments (in/out), array length must be argc
	/// @return Number of arguments processed
	int mygetopt (int argc, const char* const argv[], bool argp[] = 0);

	/// Process a variable
	/// @param arg Process argument describign the variable and type
	/// @return True if successfully processed
	bool operator() (const ParseUtil::process_arg& arg);

	/// Get listing type
	listing_type get_listing () const { return listing; }
	/// Set listing
	void set_listing (listing_type lt) { listing = lt; }
	/// Is long listing?
	bool is_verbose() const { return verbose; }
	/// Set long listing
	void set_verbose (bool vrbs) { verbose = vrbs; }

protected:
	/// Listing type
	listing_type	listing;
	/// long listing
	bool			verbose;
};

/** This enum describes the type of listing to produce
************************************************************************/
enum device_support_type {
	/// Use opc names in the INPUT/OUTPUT epics fields
	device_support_opc_name, 
	/// Use TwinCAT names in the INPUT/OUTPUT epics fields
	device_support_tc_name
};

/** Class for generatig an EPICS database record 
************************************************************************/
class epics_db_processing : 
	public epics_conversion, public split_io_support {
public:
	/// Default constructor
	epics_db_processing () {}

	/// Constructor
	/// Command line arguments will override default parameters when specified
	/// The format is the same as the arguments passed to the main program
	/// argv[0] is program name and will be ignored
	/// Processed options with epics_conversion::getopt, 
	/// split_io_support::getopt and mygetopt().
	/// @param fname Output filename
	/// @param argc Number of command line arguments
	/// @param argv List of command line arguments, same format as in main()
	/// @param argp Excluded/processed arguments (in/out), array length must be argc
	epics_db_processing (const std::stringcase& fname,
		int argc, const char* const argv[], bool argp[] = 0);

	/// Parse a command line
	/// Processed options with epics_conversion::getopt and mygetopt.
	/// @param argc Number of command line arguments
	/// @param argv List of command line arguments, same format as in main()
	/// @param argp Excluded/processed arguments (in/out), array length must be argc
	/// @return Number of arguments processed
	int getopt (int argc, const char* const argv[], bool argp[] = 0);
	/// Parse a command line
	/// The format is the same as the arguments passed to the main program
	/// argv[0] is program name and will be ignored
	/// The argp boolean array can be used to pass in a list of already
	/// processed (and to be ignored) command line arguments. This list,
	/// if supplied, must be at least argc long. Upon return, newly processed
	/// arguments are also marked as processed in this list. The arguments are:
	///
	/// /devopc: Uses OPC name in INPUT/OUTPUT field (default)
	/// /devtc:  Uses TwinCAT name in INPUT/OUTPUT fields instead of OPC
	///
	/// Command line arguments can use '-' instead of a '/'. Capitalization does
	/// not matter. getopt will only override arguments that are specifically 
	/// specified. It relies on the contructors to provide the defaults.
	/// @param argc Number of command line arguments
	/// @param argv List of command line arguments, same format as in main()
	/// @param argp Excluded/processed arguments (in/out), array length must be argc
	/// @return Number of arguments processed
	int mygetopt (int argc, const char* const argv[], bool argp[] = 0);

	/// Get device support conversion rule
	device_support_type get_device_support () const { 
		return device_support; }
	/// Set device support conversion rule
	void set_device_support (device_support_type devsup) {
		device_support = devsup; }

	/// Process a variable
	/// @param arg Process argument describign the variable and type
	/// @return True if successfully processed
	bool operator() (const ParseUtil::process_arg& arg);

protected:
	/// Process a record field of type string
	/// @param name Name of field
	/// @param val Value of field
	/// @return True if successful
	bool process_field_string (std::stringcase name, 
		std::stringcase val);
	/// Process a record field of numeric type
	/// @param name Name of field
	/// @param val Value of field
	/// @return True if successful
	bool process_field_numeric (std::stringcase name, int val);
	/// Process a record field of type string
	/// @param name Name of field
	/// @param val Value of field
	/// @return True if successful
	bool process_field_numeric (std::stringcase name, 
		std::stringcase val);
	/// Process a record field of type alarm
	/// @param name Name of field
	/// @param severity Severity of alarm
	/// @return True if successful
	bool process_field_alarm (std::stringcase name, 
		std::stringcase severity);

	/// Device support field conversion rule
	device_support_type	device_support;
};


/** @} */

}
