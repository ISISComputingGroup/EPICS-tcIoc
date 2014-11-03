#pragma once
#include "stdafx.h"

/** @file ParseUtil.h
	Header which includes utility classes for parsing. 
	It includes "ParseUtilTemplate.h"
 ************************************************************************/

/** @namespace ParseUtil
	ParseTpy name space 
 ************************************************************************/
namespace ParseUtil {

/** @defgroup parseopt Option processing
    These function and classes are used to parse command line options
 ************************************************************************/
/** @{ */

/** This class transforms a string into a standard program argument.
 ************************************************************************/
class optarg 
{
public:
	/// Default constructor
	optarg() : mysize (0), myargc (0), myargv (nullptr), myargp (nullptr) {}
	/// Constructor
	/// @param arg Option argument string
	explicit optarg (const std::stringcase& arg) 
		: mysize(0), myargc (0), myargv (nullptr), myargp (nullptr) { parse (arg); }
	/// Destructor
	~optarg() { setup (0); }
	/// Parse string argument
	/// @param arg Option argument string
	/// @return Number of processed arguments
	int parse (const std::stringcase& arg);

	/// Returns the number of arguments
	int argc() const { return myargc; }
	/// Returns the argument list
	const char* const* argv() const { return myargv; }
	/// Return the processed argument list
	const bool* argp() const { return myargp; }
	/// Return the processed argument list
	bool* argp() { return myargp; }

	/// Returns true if there all arguments are unprocessed
	bool all_done() const;

protected:
	/// Disabled copy constructor
	optarg (const optarg&);
	/// Disabled assignment operator
	optarg& operator= (const optarg&);
	/// Setup 
	void setup (int size);

	/// Size of allocated arrays
	int			mysize;
	/// Numbert of arguments
	int			myargc;
	/// Argument list
	char**		myargv;
	/// Processed argument list
	bool*		myargp;
};

/** @} */

/** @defgroup parsetpyopc OPC related functions and classes
    These function and classes are generally used to describe channel 
	properties
 ************************************************************************/
/** @{ */

/** This enum denotes the opc state.
 ************************************************************************/
enum opc_enum 
{
	/// Do not change inherited behaviour
	no_change, 
	/// Publish
	publish, 
	/// Do not publish
	silent
};

/** This map stores a list of opc properties.
 ************************************************************************/
typedef std::map<int, std::stringcase> property_map;

/** This pair stores one element of opc properties.
 ************************************************************************/
typedef std::pair<int, std::stringcase> property_el;

/** This class stores OPC properties.
************************************************************************/
class opc_list 
{
public:
	/// Default constructor
	opc_list() : opc (no_change) {}

	/// Get opc state
	opc_enum get_opc_state () const { return opc; }
	/// Set opc state 
	void set_opc_state (opc_enum state) {opc = state; }
	/// Get opc property list
	const property_map& get_properties () const { return opc_prop; }
	/// Get opc property list
	property_map& get_properties () { return opc_prop; }
	/// Get the specified property
	const property_map::const_iterator get_property (int key) const {
		return opc_prop.find (key); }

	/// Add an OPC property
	void add (const property_el& el) { opc_prop.insert (el); }
	/// Add an OPC list
	void add (const opc_list& o);

	/// Is this item published?
	bool is_published () const;
	/// Is readonly?
	bool is_readonly() const;
	/// Get string property
	bool get_property (int prop, std::stringcase& val) const;
	/// Get integer property
	bool get_property (int prop, int& val) const;
	/// Get real property
	bool get_property (int prop, double& val) const;

protected:
	/// OPC state
	opc_enum		opc;
	/// List of OPC properties
	property_map	opc_prop;
};

/** @} */

/** @defgroup parsetpyparseinfo Classes for describing a parser argument
 ************************************************************************/
/** @{ */

/** This is a class for storing bit offset and size in a structure
************************************************************************/
class bit_location
{
public:
	/// Default constructor
	bit_location () : bitoffs (0), bitsize (0) {}
	/// Constructor
	bit_location (int bo, int bs) : bitoffs (bo), bitsize (bs) {}

	/// Validity
	bool isValid() const { return (bitoffs >= 0) && (bitsize > 0); }

	/// Get bit offset 
	const int get_bit_offset () const { return bitoffs; }
	/// Set bit offset 
	void set_bit_offset  (int ofs) { bitoffs = ofs; }
	/// Get bit size
	const int get_bit_size () const { return bitsize; }
	/// Set bit size
	void set_bit_size  (int size) { bitsize = size; }

protected:
	/// bit offset where elements is stored
	int				bitoffs;
	/// size in number of bits of symbol 
	int				bitsize;
};

/** This structure holds a memory location
 ************************************************************************/
class memory_location 
{
public:
	/// Default constructor
	memory_location () : igroup(-1), ioffset (-1), bytesize (-1) {}
	/// Constructor
	/// @param ig Index group
	/// @param io Index offset
	/// @param bs Size in bytes
	memory_location (int ig, int io, int bs) 
		: igroup(ig), ioffset (io), bytesize (bs) {}
	/// Constructor
	/// @param s Definition string
	memory_location (const std::stringcase& s) 
		: igroup(-1), ioffset (-1), bytesize (-1) { set (s); }

	/// Validity
	bool isValid() const;

	/// Get IGroup
	int get_igroup () const { return igroup; }
	/// Set IGroup
	void set_igroup (int ig) { igroup = ig; }
	/// Get IOffset
	int get_ioffset () const { return ioffset; }
	/// Set IOffset
	void set_ioffset (int io) { ioffset = io; }
	/// Get BitSize
	int get_bytesize () const { return bytesize; }
	/// Set BitSize
	void set_bytesize (int bs) { bytesize = bs; }

	/// Set a sub section
	/// @param loc Bit location witin memory region
	/// @return True if section within bounds, false otherwise
	bool set_section (const bit_location& loc);

	/// Gets a string representation of a memory location
	/// @return string with format "igroup/ioffset:size", empty on error
	std::stringcase get() const;
	/// Set the memory location using a string of the form:
	/// "igroup/ioffset:size" where the size is in bytes
	/// @param s String describing memory location
	/// @return True if successful
	bool set (const std::stringcase& s); 

protected:
	/// Memory group
	int				igroup;
	/// Memory offset 
	int				ioffset;
	/// Memory size in bits
	int				bytesize;
};


/** Enumerated type to describe the process type
 ************************************************************************/
enum process_type_enum 
{
	/// Invalid type
	pt_invalid,
	/// Numeral type
	pt_int, 
	/// Floating point type
	pt_real, 
	/// Logic type
	pt_bool, 
	/// String type
	pt_string, 
	/// Enumerated type
	pt_enum, 
	/// Binary type
	pt_binary
};

/** Argument which is passed to the name/tag processing function.
************************************************************************/
class process_arg
{
public:
	/// Constructor
	/// @param loc Memory location
	/// @param vname Variable name
	/// @param pt Process type 
	/// @param o OPC list
	/// @param tname Type name
	/// @param at Atomic type
	process_arg (const memory_location& loc,
		const std::stringcase& vname, process_type_enum pt, 
		const opc_list& o, const std::stringcase& tname, bool at) 
		: name (vname), type_n (tname), opc (o), memloc (loc),
		ptype (pt), atomic (at) {}

	/// Get name
	const std::stringcase& get_name() const { return name; }
	/// Get type name 
	const std::stringcase& get_type_name() const { return type_n; }
	/// Get OPC list
	const opc_list& get_opc() const { return opc; }

	/// Get process type
	process_type_enum get_process_type () const { return ptype; }
	/// Get process type string
	std::stringcase get_process_string () const;
	/// Is atomic (or structured) type
	bool is_atomic() const { return atomic; }

	/// Get IGroup
	int get_igroup () const { return memloc.get_igroup(); }
	/// Get IOffset
	int get_ioffset () const { return memloc.get_ioffset(); }
	/// Get BitSize
	int get_bytesize () const { return memloc.get_bytesize(); }
	/// Gets a string representation of a memory location
	/// @return string with format "igroup/ioffset:size", empty on error
	std::stringcase get() const { return memloc.get(); }
	/// Gets a string representation of a PLC & memory location
	/// @return string with format "prefixigroup/ioffset:size", empty on error
	std::stringcase get_full() const;

protected:
	/// name of type
	const std::stringcase&	name;
	/// type definition
	const std::stringcase&	type_n;
	/// list of opc properties
	const opc_list&			opc;
	/// memory location
	const memory_location&	memloc;

	/// Process Type
	process_type_enum		ptype;
	/// Atomic element
	bool					atomic;
};

/** Enumerated type to describe the tag processing
************************************************************************/
enum process_tag_enum 
{
	/// Process all data types
	process_all,
	/// Process atomic data types
	process_atomic,
	/// Process structured data type (array, struct, function block)
	process_structured
};

/** Class to specify which symbols and tags/names to process
************************************************************************/
class tag_processing 
{
public:
	/// Default constructor
	tag_processing() : export_all (false), process_tags (process_all),
		no_string_tags (false) {}
	/// Constructor
	/// @param all Process all tags
	/// @param proctags Process atomic and/or structured tags
	/// @param nostring Don't process string tags
	tag_processing (bool all, process_tag_enum proctags, bool nostring = false) 
		: export_all (all), process_tags (proctags), 
		no_string_tags (nostring) {}

	/// Constructor
	/// Commaline arguments will override default parameters when specified
	/// The format is the same as the arguments passed to the main program
	/// argv[0] is program name and will be ignored
	/// @param argc Number of command line arguments
	/// @param argv List of command line arguments, same format as in main()
	/// @param argp Excluded/processed arguments (in/out), array length must be argc
	tag_processing (int argc, const char* const argv[], bool argp[] = 0)
		: export_all (false), process_tags (process_all), 
		no_string_tags (false) { getopt (argc, argv, argp); }

	/// Parse a command line
	/// The format is the same as the arguments passed to the main program
	/// argv[0] is program name and will be ignored
	/// The argp boolean array can be used to pass in a list of already
	/// processed (and to be ignored) command line arguments. This list,
	/// if supplied, must be at least argc long. Upon return, newly processed
	/// arguments are also marked as processed in this list. The arguments are:
	///
	/// /ea: Export all variables regardless of OPC setting
	/// /eo: Only export variables which are marked as OPC export (default)
	/// /ns: No string variables are processed
	/// /ys: String variables are processes (default)
	/// /pa: Call process for all types (default)
	/// /ps: Call process for simple (atomic) types only
	/// /pc: Call process for complex (structure and array) types only
	///
	/// Command line arguments can use '-' instead of a '/'. Capitalization does
	/// not matter. getopt will only override arguments that are specifically 
	/// specified. It relies on the contructors to provide the defaults.
	/// @param argc Number of command line arguments
	/// @param argv List of command line arguments, same format as in main()
	/// @param argp Excluded/processed arguments (in/out), array length must be argc
	/// @return Number of arguments processed
	int getopt (int argc, const char* const argv[], bool argp[] = 0);

	/// Get the export rule
	bool get_export_all () const { return export_all; }
	/// Set the export rule
	void set_export_all (bool all) {
		export_all = all; }
	/// Get the process rule
	process_tag_enum get_process_tags () const { return process_tags; }
	/// Set the process rule
	void set_process_tags (process_tag_enum proctags) {
		process_tags = proctags; }
	/// Get the string rule
	bool get_no_strings () const { return no_string_tags; }
	/// Set the string rule
	void set_no_strings (bool nostring) {
		no_string_tags = nostring; }

protected:
	/// Process all symbols regarless of opc publish setting
	bool			export_all;
	/// Process only atomic types
	process_tag_enum	process_tags;
	/// Don't process strings
	bool			no_string_tags;
};

/** @} */

}
