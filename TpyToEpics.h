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

/** @defgroup epicstpyutil EPICS utility functions and classes
 ************************************************************************/
/** @{ */

/** This enum describes the TwinCAT/opc to EPICS conversion rule
	@brief Conversion rules for TC/EPICS
 ************************************************************************/
enum class tc_epics_conv {
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
     @brief Case conversion rule enum
************************************************************************/
enum class case_type {
	/// Preserve the case
	preserve, 
	/// Convert to upper case
	upper, 
	/// Convert to lower case
	lower
};

/** Epics channel conversion arguments
    Epics channels are generated from opc through a conversion rule
	@brief Epics conversion
************************************************************************/
class epics_conversion : public ParseUtil::replacement_rules, 
					     public ParseUtil::substitution_list {
public:
	/// Default constructor
	epics_conversion() noexcept = default;
	/// Constructor
	/// @param caseconv Case conversion specification
	/// @param noindex Eliminate array indices '[n]' with '_n'
	epics_conversion (case_type caseconv, bool noindex) noexcept
		: case_epics_names (caseconv), no_array_index (noindex) {}
	/// Constructor
	/// @param epics_conv Epics conversion rule
	/// @param caseconv Case conversion specification
	/// @param noldot Eliminate leading dot in a name
	/// @param noindex Eliminate array indices '[n]' with '_n'
	epics_conversion (tc_epics_conv epics_conv, case_type caseconv, 
		bool noldot, bool noindex) noexcept
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
		{ getopt (argc, argv ,argp);}
	/// Destructor
	virtual ~epics_conversion() = default;

	/// Parse a command line
	/// The format is the same as the arguments passed to the main program
	/// argv[0] is program name and will be ignored
	/// The argp boolean array can be used to pass in a list of already
	/// processed (and to be ignored) command line arguments. This list,
	/// if supplied, must be at least argc long. Upon return, newly processed
	/// arguments are also marked as processed in this list. The arguments are:
	///
	/// /fs 'filename': Reads a substitution file and keeps defined or published channels
	/// /fi 'filename': Reads a substitution file and keeps only defined channels
	/// /fa 'filename': Reads a substitution file and keeps all channels
	/// /nd: Eliminates leading dot in channel name (default)
	/// /yd: Leaves leading dot in channel name
	/// /rn: Does not apply any special conversion rules
	/// /rd: Replaces dots with underscores in channel names
	/// /rl: LIGO standard conversion rule (default)
	/// /rv: LIGO vacuum channel conversion rule
	/// /cp: Preserve case in EPICS channel names
	/// /cu: Force upper case in EPICS channel names (default)
	/// /cl: Force lower case in EPICS channel names
	/// /ni: Replaces array brackets with underscore (default)
	/// /yi: Leave array indices as is
	/// /p 'name': Include a prefix of 'name' for every channel (defaults to no prefix) 
	///
	/// The channel name conversions are processed in the order specified above. 
	/// Replacement rules, if specified, are applied after the substitution file has 
	/// been processed and before all other steps.
	/// 
	/// Command line arguments can use '-' instead of a '/'. Capitalization does
	/// not matter. getopt will only override arguments that are specifically 
	/// specified. It relies on the contructors to provide the defaults.
	/// @param argc Number of command line arguments
	/// @param argv List of command line arguments, same format as in main()
	/// @param argp Excluded/processed arguments (in/out), array length must be argc
	/// @return Number of arguments processed
	virtual int getopt (int argc, const char* const argv[], bool argp[] = 0);

	/// Get the conversion rule
	tc_epics_conv get_conversion_rule () const noexcept { return conv_rule; }
	/// Set the conversion rule
	void set_conversion_rule (tc_epics_conv epics_conv) noexcept {
		conv_rule = epics_conv; }
	/// Get the conversion rule
	case_type get_case_rule () const noexcept { return case_epics_names; }
	/// Set the conversion rule
	void set_case_rule (case_type epics_conv) noexcept {
		case_epics_names = epics_conv; }
	/// Get the leadin dot rule
	bool get_dot_rule () const noexcept { return no_leading_dot; }
	/// Set the leading dot rule
	void set_dot_rule (bool noldot) noexcept {
		no_leading_dot = noldot; }
	/// Get the array index rule
	bool get_array_rule () const noexcept { return no_array_index; }
	/// Set the array conversion rule
	void set_array_rule (bool noindex) noexcept {
		no_array_index = noindex; }
	/// Get the channel prefix
	std::stringcase get_prefix() const { return prefix; }
	/// Set the channel prefix
	void set_prefix(const std::stringcase& pvPrefix) {
		prefix = pvPrefix;
	}
	/// Converts a TwinCAT or OPC name to an EPICS channel name
	/// @param name TwinCAT/opc name
	/// @param published This variable is published
	/// @param subst Optional substitution return, when non null
	/// @return EPICS name
	virtual std::string to_epics (const std::stringcase& name, bool published = true,
		const ParseUtil::substitution** subst = nullptr) const;

protected:
	/// Conversion rule
	tc_epics_conv	conv_rule = tc_epics_conv::ligo_std;
	/// Case conversion rule
	case_type		case_epics_names = case_type::upper;
	/// Leading dot conversion rule
	bool			no_leading_dot = true;
	/// Array index conversion rule
	bool			no_array_index = true;
	/// Prefix to apply to all EPICS names
	std::stringcase		prefix; 
};

/** Split file IO support
    Output can be split in multiple files if the number of channels
    exceeds the maximum specified for a file
	@brief Split IO support
************************************************************************/
class split_io_support {
public:
	/// Default constructor
	split_io_support() noexcept = default;

	/// Constructor
	explicit split_io_support (const std::stringcase& fname, 
		bool split = false, int max = 0) 
		: error (false), split_io (split), split_n (max) 
	{ set_filename (fname); }
	/// Constructor
	/// Command line arguments will override default parameters when specified
	/// The format is the same as the arguments passed to the main program
	/// argv[0] is program name and will be ignored
	/// @param fname Filename for output
	/// @param argc Number of command line arguments
	/// @param argv List of command line arguments, same format as in main()
	/// @param argp Excluded/processed arguments (in/out), array length must be argc
	split_io_support (const std::stringcase& fname,
		int argc, const char* const argv[], bool argp[] = 0) 
		: error (false) { 
		getopt (argc, argv, argp); set_filename (fname); }
	/// Destructor
	virtual ~split_io_support ();
	/// Copy constructor
	/// File pointers will be moved over and the original ones will become invalid.
	split_io_support (const split_io_support&);
	/// Move constructor
	split_io_support(split_io_support&&) = delete;
	/// Copy assignment
	/// File pointers will be moved over and the original ones will become invalid.
	split_io_support& operator= (const split_io_support&);
	/// Move assignment
	split_io_support& operator= (split_io_support&&) = delete;

	/// Return error
	virtual bool operator! () const noexcept { return error; }

	/// Increase the channel number
	/// @param readonly Inidcates if channel is readonly
	/// @return True if no error
	bool increment (bool readonly);
	/// Flush contents of output files
	virtual void flush() noexcept;

	/// Get output file
	FILE* get_file () const noexcept {return outf; }
	/// Get output filename
	const std::stringcase& get_filename () const noexcept
	{ return outfilename; }
	/// Is output split?
	bool is_split() const noexcept { return split_io; }
	/// Maximum of channels per file
	int get_max() const noexcept { return split_n; }

	/// Get number of processed channels
	int get_processed_total() const noexcept { return rec_num; }
	/// Get number of processed readonly channels
	int get_processed_readonly() const noexcept { return rec_num_in; }
	/// Get number of processed input/ouput channels
	int get_processed_io() const noexcept { return rec_num_io; }

protected:
	/// Set output filename
	virtual void set_filename (const std::stringcase& fname);
	/// Close files
	virtual void close() noexcept;
	/// set split
	void set_split (bool split) noexcept { split_io = split; }
	/// Set maximum of channels per file
	void set_max (int max) noexcept { split_n = (max <= 0) ? 0 : max; }

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
	virtual int getopt (int argc, const char* const argv[], bool argp[] = 0);

	/// Error 
	bool			error = true;
	
	/// Output filename
	std::stringcase		outfilename;
	/// Split output into read only channels and input/output channels
	bool			split_io = false;
	/// Maximum number of channels per file; 0 indicates no limit
	int				split_n = 0;
	/// Output file
	mutable FILE*	outf = stdout;
	/// Output file for read only channels
	mutable FILE*	outf_in = nullptr;
	/// Output file for input/output channels
	mutable FILE*	outf_io = nullptr;

	/// Current number of processed channels (records)
	int				rec_num = 0;
	/// Current number of processed read only channels (records)
	int				rec_num_in = 0;
	/// Current number of processed input/output channels (records)
	int				rec_num_io = 0;
	/// Current file number of processed read only channels (records)
	int				file_num_in = 1;
	/// Current file number of processed input/output channels (records)
	int				file_num_io = 1;

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

/// enum for file io
enum class io_filestat {
	/// file is closed
	closed,
	// file is open for reading
	read,
	// file is open for writing
	write
};

/** Multi file IO support
    Supports a directory argument and opens files within
	@brief Multiple IO support
************************************************************************/
class multi_io_support {
public:
	/// Default constructor
	multi_io_support() = default;

	/// Constructor
	explicit multi_io_support (const std::stringcase& dname)
		{ set_outdirname (dname); set_indirname (dname); }
	/// Constructor
	/// Command line arguments will override default parameters when specified
	/// The format is the same as the arguments passed to the main program
	/// argv[0] is program name and will be ignored
	/// @param dname Name of output directory
	/// @param argc Number of command line arguments
	/// @param argv List of command line arguments, same format as in main()
	/// @param argp Excluded/processed arguments (in/out), array length must be argc
	multi_io_support (const std::stringcase& dname,
		int argc, const char* const argv[], bool argp[] = 0)
		{ getopt (argc, argv, argp); set_outdirname (dname); set_indirname (dname); }
	/// Destructor
	virtual ~multi_io_support () {close(); }

	/// Return error
	bool operator! () const;
	/// Open file for reading/writing
	virtual bool open (const std::stringcase& fname, const std::stringcase& io = "w",
			bool superrmsg = false);
	/// Close file
	virtual void close() noexcept;
	/// Get file handle
	FILE* get_file () const noexcept {return filehandle; }

	/// Set output directory name
	void set_outdirname (const std::stringcase& dname);
	/// Get output directory name
	const std::stringcase& get_outdirname () const noexcept {
		return outdirname; }
	/// Set input directory name
	void set_indirname (const std::stringcase& dname);
	/// Get input directory name
	const std::stringcase& get_indirname () const noexcept {
		return indirname; }

	/// Get full filename
	const std::stringcase& get_filename () const noexcept {
		return filename; }
	/// Reading
	io_filestat fileread () const noexcept { return filestat; }

	/// Get number of read files
	int get_filein_total() const noexcept { return file_num_in; }
	/// Get number of written files
	int get_fileout_total() const noexcept { return file_num_out; }

protected:
	/// Parse a command line
	/// The format is the same as the arguments passed to the main program
	/// argv[0] is program name and will be ignored
	/// The argp boolean array can be used to pass in a list of already
	/// processed (and to be ignored) command line arguments. This list,
	/// if supplied, must be at least argc long. Upon return, newly processed
	/// arguments are also marked as processed in this list. The arguments are:
	///
	/// Command line arguments can use '-' instead of a '/'. Capitalization does
	/// not matter. getopt will only override arguments that are specifically 
	/// specified. It relies on the contructors to provide the defaults.
	/// @param argc Number of command line arguments
	/// @param argv List of command line arguments, same format as in main()
	/// @param argp Excluded/processed arguments (in/out), array length must be argc
	/// @return Number of arguments processed
	virtual int getopt (int argc, const char* const argv[], bool argp[] = 0) {
		return 0; }

	/// Directory name
	std::stringcase	outdirname;
	/// Directory name
	std::stringcase	indirname;
	/// Current filename
	std::stringcase	filename;
	/// reading or writing?
	io_filestat		filestat = io_filestat::closed;
	/// Output file
	mutable FILE*	filehandle = nullptr;

	/// Current file number of processed read only channels (records)
	int				file_num_in = 0;
	/// Current file number of processed input/output channels (records)
	int				file_num_out = 0;
};


/** This enum describes the type of listing to produce
     @brief Listing type enum
************************************************************************/
enum class listing_type {
	/// Standard listing using TwinCAT/OPC names
	standard, 
	/// Autoburt listing
	autoburt,
	/// LIGO DAQ ini listing
	daqini
};

/** Class for generatig a channel list
	@brief List processing
************************************************************************/
class epics_list_processing : 
	public epics_conversion, public split_io_support {
public:
	/// Default constructor
	epics_list_processing() noexcept = default;
	/// Constructor
	/// @param ltype Type of listing
	/// @param ll long listing
	explicit epics_list_processing (listing_type ltype, bool ll = false) noexcept
		: listing (ltype), verbose (ll) {}
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

	/// Default destructor
	virtual ~epics_list_processing() = default;

	/// Parse a command line
	/// Processed options with epics_conversion::getopt and mygetopt().
	/// @param argc Number of command line arguments
	/// @param argv List of command line arguments, same format as in main()
	/// @param argp Excluded/processed arguments (in/out), array length must be argc
	/// @return Number of arguments processed
	int getopt (int argc, const char* const argv[], bool argp[] = 0) override;
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
	/// /li: Generates a LIGO DAQ ini file
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
	virtual bool operator() (const ParseUtil::process_arg& arg);

	/// Get listing type
	listing_type get_listing () const noexcept { return listing; }
	/// Set listing
	void set_listing (listing_type lt) noexcept { listing = lt; }
	/// Is long listing?
	bool is_verbose() const noexcept { return verbose; }
	/// Set long listing
	void set_verbose (bool vrbs) noexcept { verbose = vrbs; }

protected:
	/// Listing type
	listing_type	listing = listing_type::standard;
	/// long listing
	bool			verbose = false;
};

/** This enum describes the type of macros to produce
 ************************************************************************/
enum class macrofile_type {
	/// Include all fields and error messages
	all, 
	/// Include all fields
	fields,
	/// Include error messages
	errors
};

/** This structure describes a field
    @brief Macro information
************************************************************************/
struct macro_info {
	/// Default constructor
	macro_info() noexcept = default;

	/// Process Type
	ParseUtil::process_type_enum	ptype = ParseUtil::process_type_enum::pt_invalid;
	/// name of type
	std::stringcase			name;
	/// type definition
	std::stringcase			type_n;
	/// readonly
	bool					readonly = false;
};

/** A list of fields
 ************************************************************************/
using macro_list = std::vector<macro_info>;

/** This structure describes a record/struct
    @brief Macro record
************************************************************************/
struct macro_record {
	macro_record() noexcept = default;
	/// name of structure
	macro_info				record;
	/// is an ErrorStruct
	bool					iserror = false;
	/// contains an ErrorStruct
	bool					haserror = false;
	/// index if fileds list to ErrorStruct
	int						erroridx = -1;
	/// List of fields
	macro_list				fields;
	/// name of upper level structure
	macro_info				back;
};

/** A stack of records/structs
 ************************************************************************/
using macro_stack = std::stack<macro_record>;

/** A set of filenames
 ************************************************************************/
using filename_set = std::unordered_set<std::stringcase>;

/** Class for generatig macro files to be used by medm
	@brief Macro file processing
************************************************************************/
class epics_macrofiles_processing : 
	public epics_conversion, public multi_io_support {
public:
	/// Type name identifying an error struct ("ErrorStruct")
	static const std::stringcase errorstruct;
	/// TwinCAT 2.11: File extension identifying a list of error msgs ("_Errors.exp")
	static const std::stringcase errorlistext2;
	/// TwinCAT 2.11: Regular expression to match the entire error record defintition
	static const std::regex errormatchregex2;
	/// TwinCAT 3.1: File extension identifying a list of error msgs ("_Errors.TcGVL")
	static const std::stringcase errorlistext31;
	/// TwinCAT 3.1: Regular expression to match the entire error record defintition
	static const std::regex errormatchregex31;
	/// TwinCAT 2.11/3.1: Regular expression to search for the actual error messages
	static const std::regex errorsearchregex;

	/// Default constructor
	epics_macrofiles_processing() noexcept = default;
	/// Constructor
	/// @param mt Type of macro
	explicit epics_macrofiles_processing (macrofile_type mt) : macros (mt) {}
	/// Constructor
	/// Command line arguments will override default parameters when specified
	/// The format is the same as the arguments passed to the main program
	/// argv[0] is program name and will be ignored
	/// Processed options with epics_conversion::getopt, 
	/// multi_io_support::getopt and mygetopt().
	/// @param pname PLC name
	/// @param dname Directory name
	/// @param tcat3 True if we are processing TwinCAT 3.1 files
	/// @param argc Number of command line arguments
	/// @param argv List of command line arguments, same format as in main()
	/// @param argp Excluded/processed arguments (in/out), array length must be argc
	epics_macrofiles_processing (const std::stringcase& pname, 
		const std::stringcase& dname, bool tcat3,
		int argc, const char* const argv[], bool argp[] = 0);

	/// Destructor
	virtual ~epics_macrofiles_processing() { flush(); }
	/// flush all pending prcossing
	virtual void flush() noexcept;

	/// Parse a command line
	/// Processed options with epics_conversion::getopt and mygetopt().
	/// @param argc Number of command line arguments
	/// @param argv List of command line arguments, same format as in main()
	/// @param argp Excluded/processed arguments (in/out), array length must be argc
	/// @return Number of arguments processed
	int getopt (int argc, const char* const argv[], bool argp[] = 0) override;
	/// Parse a command line
	/// The format is the same as the arguments passed to the main program
	/// argv[0] is program name and will be ignored
	/// The argp boolean array can be used to pass in a list of already
	/// processed (and to be ignored) command line arguments. This list,
	/// if supplied, must be at least argc long. Upon return, newly processed
	/// arguments are also marked as processed in this list. The arguments are:
	///
	/// /mf: Generate a macro file for each structure describing all fields
	/// /me: Generate a macro file for each structure describing the error messages
	/// /ma: Generate a macro file for each structure describing fields and errors (default)
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
	virtual bool operator() (const ParseUtil::process_arg& arg);

	/// Get listing type
	macrofile_type get_macrofile_type () const noexcept { return macros; }
	/// Set listing
	void set_macrofile_type (macrofile_type m) noexcept { macros = m; }

	/// Set PLC name
	void set_plcname (const std::stringcase& name) {
		plcname = name; }
	/// Get PLC name
	const std::stringcase& get_plcname () const noexcept {
		return plcname; }

	/// Is this twincat 3?
	bool is_twincat3() const noexcept { return isTwinCAT3; }
	/// Set twincat 3 version?
	void set_twincat3 (bool tcat3 = true) noexcept {isTwinCAT3 = tcat3; }

	/// Translate epics name to filename
	std::stringcase to_filename (const std::stringcase& epicsname);

	/// Get number of processed channels
	int get_processed_total() const noexcept { return rec_num; }

protected:
	/// Process top of stack
	bool process_record (const macro_record& mrec, int level = 0);

	/// Listing type
	macrofile_type	macros = macrofile_type::all;
	/// PLC name
	std::stringcase	plcname;
	/// TwinCAT version
	bool			isTwinCAT3 = false;
	/// Processing stack
	macro_stack		procstack;
	/// Current number of processed channels (records)
	int				rec_num = 0;
	/// set of missing input files
	filename_set	missing;
};

/** This enum describes the type of device support to use
	@brief Device support enum
************************************************************************/
enum class device_support_type {
	/// Use opc names in the INPUT/OUTPUT epics fields
	opc_name, 
	/// Use TwinCAT names in the INPUT/OUTPUT epics fields
	tc_name
};

/** This enum describes the type of string record to use
	@brief String support enum
************************************************************************/
enum class string_support_type {
	/// Use standard strings (stringin/stringout)
	short_string,
	/// Use long strings (lsi/lso)
	long_string,
	// Use long or short strings depending on size
	vary_string
};

/** This enum describes the type of int record to use
	@brief Integer support enum
************************************************************************/
enum class int_support_type {
	/// Use standard 32-bit integers (longin/longout)
	int_32,
	/// Use 64-bit integers (int64in/int64out)
	int_64,
	// Use 32 or 64-bit integers depending on size
	int_auto
};

/** Class for generatig an EPICS database record
	@brief pics database record processing
************************************************************************/
class epics_db_processing : 
	public epics_conversion, public split_io_support {
public:
	/// Default constructor
	epics_db_processing() noexcept = default;

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

	virtual ~epics_db_processing() = default;

	/// Parse a command line
	/// Processed options with epics_conversion::getopt and mygetopt.
	/// @param argc Number of command line arguments
	/// @param argv List of command line arguments, same format as in main()
	/// @param argp Excluded/processed arguments (in/out), array length must be argc
	/// @return Number of arguments processed
	virtual int getopt (int argc, const char* const argv[], bool argp[] = 0);
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
	/// /ss: Use standard strings (stringin/stringout)
	/// /sl: Use long strings (lsi/lso)
	/// /sd: Use long or short strings depending on size (default)
	///
	/// /is: Use standard 32-bit integers (longin/longout)
	/// /il: Use 64-bit integers (int64in/int64out)
	/// /id: Use 32 or 64-bit integers depending on size (default)
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
	device_support_type get_device_support () const noexcept {
		return device_support; }
	/// Set device support conversion rule
	void set_device_support (device_support_type devsup) noexcept {
		device_support = devsup; }

	/// Get string support conversion rule
	string_support_type get_string_support() const noexcept {
		return string_support; }
	/// Set string support conversion rule
	void set_string_support(string_support_type strsup) noexcept {
		string_support = strsup; }

	/// Get integer support conversion rule
	int_support_type get_int_support() const noexcept {
		return int_support;	}
	/// Set integer support conversion rule
	void set_int_support(int_support_type intsup) noexcept {
		int_support = intsup; }

	/// Process a variable
	/// @param arg Process argument describign the variable and type
	/// @return True if successfully processed
	virtual bool operator() (const ParseUtil::process_arg& arg) noexcept;

protected:
	/// Process a record field of type string
	/// @param name Name of field
	/// @param val Value of field
	/// @param maxlen Maximum length of value field
	/// @return True if successful
	bool process_field_string (std::stringcase name, 
		std::stringcase val, int maxlen = 256) noexcept;
	/// Process a record field of numeric type
	/// @param name Name of field
	/// @param val Value of field
	/// @return True if successful
	bool process_field_numeric (std::stringcase name, int val) noexcept;
	/// Process a record field of numeric type
	/// @param name Name of field
	/// @param val Value of field
	/// @return True if successful
	bool process_field_numeric (std::stringcase name, double val) noexcept;
	/// Process a record field of type string
	/// @param name Name of field
	/// @param val Value of field
	/// @return True if successful
	bool process_field_numeric (std::stringcase name, 
		std::stringcase val) noexcept;
	/// Process a record field of type alarm
	/// @param name Name of field
	/// @param severity Severity of alarm
	/// @return True if successful
	bool process_field_alarm (std::stringcase name, 
		std::stringcase severity) noexcept;

	/// Device support field conversion rule
	device_support_type	device_support = device_support_type::tc_name;
	/// String support field conversion rule
	string_support_type string_support = string_support_type::vary_string;
	/// Integer support field conversion rule
	int_support_type int_support = int_support_type::int_auto;
};


/** @} */

}
