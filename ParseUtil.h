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


/** @defgroup parseutilities Parser utility functions and classes
 ************************************************************************/
/** @{ */

/** Table of replacement rules
 ************************************************************************/
using replacement_table = std::map <std::stringcase, std::stringcase>;

/** Epics channel conversion arguments
    Epics channels are generated from opc through a conversion rule
	@brief Replacement rules
 ************************************************************************/
class replacement_rules {
public:
	/// Default constructor
	explicit replacement_rules (bool rec = true) noexcept : recursive (rec) {}
	/// Constructor
	explicit replacement_rules (const replacement_table& t, bool rec = true)
		: table (t), recursive(rec) {}
	/// Constructor
	/// @param s Rule string of the form VAR1=VAL1,VAR2=VAL2,...
	/// @param alias Alias name
	explicit replacement_rules(const std::stringcase& s, const std::stringcase alias = "")	{
		parse_rules(s, alias);	}
	/// Clear
	void clear() { table.clear(); }
	/// Add a rule
	void add_rule (const std::stringcase& var, const std::stringcase& val) {
		table[var] = val; }
	/// set table
	void set_rule_table (const replacement_table& t) {
		table = t; }
	/// get table
	replacement_table& get_rule_table() noexcept {
		return table; }
	/// get table
	const replacement_table& get_rule_table() const noexcept {
		return table; }
	/// replace
	std::stringcase apply_replacement_rules (const std::stringcase& s) const;
	/// Has rules
	bool HasRules() const noexcept { return !table.empty(); }

	/// Is recursive?
	bool is_recursive() const noexcept { return recursive; }
	/// Set recursive
	void set_recursive (bool rec) noexcept {
		recursive = rec; }

	/// Parse a rule string
	/// @param s Rule string of the form VAR1=VAL1,VAR2=VAL2,...
	/// @param alias Alias name
	/// @return True if proper rule string
	bool parse_rules(const std::stringcase& s, const std::stringcase alias = "");
	/// Get the string representing the rules
	std::stringcase get_rule_string() const;

	/// prefix for replacement rule: ${
	inline static const char* const prefix = "${";
	/// suffix for replacement rule: }
	inline static const char* const suffix = "}";
protected:
	/// Replacement table
	replacement_table		table;
	/// Recusrsive replacement
	bool					recursive = true;
};


/** This class transforms a string into a standard program argument.
    @brief Optional arguments
************************************************************************/
class optarg 
{
public:
	/// Default constructor
	optarg() noexcept = default;
	/// Constructor
	/// @param arg Option argument string
	explicit optarg (const std::stringcase& arg) { parse (arg); }
	/// Destructor
	~optarg() { setup (0); }
	/// Disabled copy constructor
	optarg(const optarg&) = delete;
	/// Enable move constructor
	optarg(optarg&&) = default;
	/// Disabled assignment operator
	optarg& operator= (const optarg&) = delete;
	/// Enable move operator
	optarg& operator= (optarg&&) = default;


	/// Parse string argument
	/// @param arg Option argument string
	/// @return Number of processed arguments
	int parse (const std::stringcase& arg);

	/// Returns the number of arguments
	int argc() const noexcept { return myargc; }
	/// Returns the argument list
	const char* const* argv() const noexcept { return myargv.get(); }
	/// Return the processed argument list
	const bool* argp() const noexcept { return myargp.get(); }
	/// Return the processed argument list
	bool* argp() noexcept { return myargp.get(); }

	/// Returns true if there all arguments are unprocessed
	bool all_done() const noexcept;

protected:
	/// Setup 
	void setup (size_t size) noexcept;

	/// Size of allocated arrays
	size_t		mysize = 0;
	/// Numbert of arguments
	int			myargc = 0;
	/// Argument list
	std::unique_ptr<char*[]> myargv;
	//char**		myargv;
	/// Processed argument list
	std::unique_ptr<bool[]> myargp;
	//bool*		myargp;
};


/** This enum denotes the opc state.
     @brief OPC state enum
************************************************************************/
enum class opc_enum 
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
using property_map = std::map<int, std::stringcase>;

/** This pair stores one element of opc properties.
 ************************************************************************/
using property_el = std::pair<int, std::stringcase>;

/** This class stores OPC properties.
	@brief OPC list
************************************************************************/
class opc_list 
{
public:
	/// Default constructor
	opc_list() noexcept = default;
	/// Constructor
	opc_list (opc_enum state, const property_map& map) 
		: opc(state), opc_prop(map) {}

	/// Get opc state
	opc_enum get_opc_state () const noexcept { return opc; }
	/// Set opc state 
	void set_opc_state (opc_enum state) noexcept {opc = state; }
	/// Get opc property list
	const property_map& get_properties () const noexcept { return opc_prop; }
	/// Get opc property list
	property_map& get_properties () noexcept { return opc_prop; }
	/// Get the specified property
	const property_map::const_iterator get_property (int key) const {
		return opc_prop.find (key); }

	/// Add an OPC property
	void add (const property_el& el) { opc_prop.insert (el); }
	/// Add an OPC list
	void add (const opc_list& o);

	/// Is this item published?
	bool is_published () const noexcept;
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
	opc_enum		opc = opc_enum::no_change;
	/// List of OPC properties
	property_map	opc_prop;
};


/** This is a class for storing a variable name and an alias
	@brief Variable name
************************************************************************/
class variable_name
{
public:
	/// Default constructor
	variable_name() noexcept = default;
	/// Constructor
	explicit variable_name (const std::stringcase& n) : name (n), alias (n) {}
	/// Constructor
	explicit variable_name (const char* s) : name(s ? s : ""), alias(s ? s : "") {}
	/// Constructor
	variable_name (const std::stringcase& n, const std::stringcase& a) 
		: name (n), alias (a) {}

	/// Get name
	const std::stringcase& get_name() const noexcept { return name; }
	/// Get name
	std::stringcase& get_name() noexcept { return name; }
	/// Get alias
	const std::stringcase& get_alias() const noexcept { return alias; }
	/// Get alias
	std::stringcase& get_alias() noexcept { return alias; }
	/// Set name & alias
	void set (const std::stringcase& n) { 
		name = n; alias = n; }
	/// Set name & alias
	void set (const std::stringcase& n, const std::stringcase& a) { 
		name = n; alias = a; }
	/// Append
	void append (const std::stringcase& n, const std::stringcase& sep = ".");
	/// Append
	void append (const std::stringcase& n, const opc_list& opc, 
		const std::stringcase& sep = ".");

protected:
	/// variable name
	std::stringcase		name;
	/// alias name
	std::stringcase		alias;
};

	
	
/** This is a class for storing bit offset and size in a structure
	@brief Bit location
************************************************************************/
class bit_location
{
public:
	/// Default constructor
	bit_location() noexcept = default;
	/// Constructor
	bit_location (int bo, int bs) noexcept : bitoffs (bo), bitsize (bs) {}

	/// Validity
	bool isValid() const noexcept { return (bitoffs >= 0) && (bitsize > 0); }

	/// Get bit offset 
	const int get_bit_offset () const noexcept { return bitoffs; }
	/// Set bit offset 
	void set_bit_offset  (int ofs) noexcept { bitoffs = ofs; }
	/// Get bit size
	const int get_bit_size () const noexcept { return bitsize; }
	/// Set bit size
	void set_bit_size  (int size) noexcept { bitsize = size; }

protected:
	/// bit offset where elements is stored
	int				bitoffs = 0;
	/// size in number of bits of symbol 
	int				bitsize = 0;
};

/** This structure holds a memory location
    @brief Memory location
************************************************************************/
class memory_location 
{
public:
	/// Default constructor
	memory_location() noexcept = default;
	/// Constructor
	/// @param ig Index group
	/// @param io Index offset
	/// @param bs Size in bytes
	memory_location (int ig, int io, int bs) noexcept
		: igroup(ig), ioffset (io), bytesize (bs) {}
	/// Constructor
	/// @param s Definition string
	memory_location (const std::stringcase& s) noexcept { set (s); }

	/// Validity
	bool isValid() const noexcept;

	/// Get IGroup
	int get_igroup () const noexcept { return igroup; }
	/// Set IGroup
	void set_igroup (int ig) noexcept { igroup = ig; }
	/// Get IOffset
	int get_ioffset () const noexcept { return ioffset; }
	/// Set IOffset
	void set_ioffset (int io) noexcept { ioffset = io; }
	/// Get BitSize
	int get_bytesize () const noexcept { return bytesize; }
	/// Set BitSize
	void set_bytesize (int bs) noexcept { bytesize = bs; }

	/// Set a sub section
	/// @param loc Bit location witin memory region
	/// @return True if section within bounds, false otherwise
	bool set_section (const bit_location& loc) noexcept;

	/// Gets a string representation of a memory location
	/// @return string with format "igroup/ioffset:size", empty on error
	std::stringcase get() const;
	/// Set the memory location using a string of the form:
	/// "igroup/ioffset:size" where the size is in bytes
	/// @param s String describing memory location
	/// @return True if successful
	bool set (const std::stringcase& s) noexcept;

protected:
	/// Memory group
	int				igroup = -1;
	/// Memory offset 
	int				ioffset = -1;
	/// Memory size in bits
	int				bytesize = -1;
};


/** Enumerated type to describe the process type
	@brief Process type 
 ************************************************************************/
enum class process_type_enum 
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
	@brief Arguments for processing
************************************************************************/
class process_arg
{
public:
	/// Constructor
	/// @param vname Variable name
	/// @param pt Process type 
	/// @param o OPC list
	/// @param tname Type name
	/// @param at Atomic type
	process_arg (const variable_name& vname, process_type_enum pt, 
		const opc_list& o, const std::stringcase& tname, bool at)
		: name (vname), type_n (tname), opc (o), ptype (pt), atomic (at) { 
		deduce_size(); }
	/// Destructor
	virtual ~process_arg() = default;
	/// Copy constructor
	process_arg(const process_arg&) = default;
	/// Move constructor
	process_arg(process_arg&&) = delete;
	/// Assignment operator
	process_arg& operator=(const process_arg&) = delete;
	/// Move  operator
	process_arg& operator=(process_arg&&) = delete;

	/// Get variable
	const variable_name& get_var() const noexcept { return name; }
	/// Get name
	const std::stringcase& get_name() const noexcept { return name.get_name(); }
	/// Get alias
	const std::stringcase& get_alias() const noexcept { return name.get_alias(); }
	/// Get type name 
	const std::stringcase& get_type_name() const noexcept { return type_n; }
	/// Get OPC list
	const opc_list& get_opc() const noexcept { return opc; }

	/// Get process type
	process_type_enum get_process_type () const noexcept { return ptype; }
	/// Get process type string
	std::stringcase get_process_string () const;
	/// Is atomic (or structured) type
	bool is_atomic() const noexcept { return atomic; }
	/// Get string length
	int get_size() const noexcept { return size; }

	/// Gets a string representation of a PLC & memory location
	/// @return string with format "prefixigroup/ioffset:size", empty on error
	virtual std::stringcase get_full() const = 0;

protected:
	/// name of type
	const variable_name&	name;
	/// type definition
	const std::stringcase&	type_n;
	/// list of opc properties
	const opc_list&			opc;
	/// Process Type
	process_type_enum		ptype = process_type_enum::pt_invalid;
	/// Atomic element
	bool					atomic = false;
	// Length of string
	int						size = 0;

	/// deduce string and int length from type
	void deduce_size();
};

/** Argument which is passed to the name/tag processing function.
	@brief Arguments for processing
************************************************************************/
class process_arg_tc : public process_arg
{
public:
	/// Constructor
	/// @param loc Memory location
	/// @param vname Variable name
	/// @param pt Process type 
	/// @param o OPC list
	/// @param tname Type name
	/// @param at Atomic type
	process_arg_tc (const memory_location& loc,
		const variable_name& vname, process_type_enum pt,
		const opc_list& o, const std::stringcase& tname, bool at)
		: process_arg (vname, pt, o, tname, at), memloc(loc) {}

	/// Get IGroup
	int get_igroup() const noexcept { return memloc.get_igroup(); }
	/// Get IOffset
	int get_ioffset() const noexcept { return memloc.get_ioffset(); }
	/// Get BitSize
	int get_bytesize() const noexcept { return memloc.get_bytesize(); }
	/// Gets a string representation of a memory location
	/// @return string with format "igroup/ioffset:size", empty on error
	std::stringcase get() const { return memloc.get(); }
	/// Gets a string representation of a PLC & memory location
	/// @return string with format "prefixigroup/ioffset:size", empty on error
	std::stringcase get_full() const override;

protected:
	/// memory location
	const memory_location&	memloc;
};

/** Enumerated type to describe the tag processing
	@brief Tag processing enum
************************************************************************/
enum class process_tag_enum 
{
	/// Process all data types
	all,
	/// Process atomic data types
	atomic,
	/// Process structured data type (array, struct, function block)
	structured
};

/** Class to specify which symbols and tags/names to process
	@brief Tag processing selection
************************************************************************/
class tag_processing 
{
public:
	/// Default constructor
	tag_processing() noexcept = default;
	/// Constructor
	/// @param all Process all tags
	/// @param proctags Process atomic and/or structured tags
	/// @param nostring Don't process string tags
	tag_processing (bool all, process_tag_enum proctags, bool nostring = false) noexcept
		: export_all (all), process_tags (proctags), no_string_tags (nostring) {}

	/// Constructor
	/// Commaline arguments will override default parameters when specified
	/// The format is the same as the arguments passed to the main program
	/// argv[0] is program name and will be ignored
	/// @param argc Number of command line arguments
	/// @param argv List of command line arguments, same format as in main()
	/// @param argp Excluded/processed arguments (in/out), array length must be argc
	tag_processing (int argc, const char* const argv[], bool argp[] = 0) noexcept
		: export_all (false), process_tags (process_tag_enum::all), 
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
	int getopt (int argc, const char* const argv[], bool argp[] = 0) noexcept;

	/// Get the export rule
	bool get_export_all () const noexcept { return export_all; }
	/// Set the export rule
	void set_export_all (bool all) noexcept {
		export_all = all; }
	/// Get the process rule
	process_tag_enum get_process_tags () const noexcept { return process_tags; }
	/// Set the process rule
	void set_process_tags (process_tag_enum proctags) noexcept {
		process_tags = proctags; }
	/// Get the string rule
	bool get_no_strings () const noexcept { return no_string_tags; }
	/// Set the string rule
	void set_no_strings (bool nostring) noexcept {
		no_string_tags = nostring; }

protected:
	/// Process all symbols regarless of opc publish setting
	bool			export_all = false;
	/// Process only atomic types
	process_tag_enum	process_tags = process_tag_enum::all;
	/// Don't process strings
	bool			no_string_tags = false;
};


/** Class to specify a channel substitution
	@brief Channel substitution
************************************************************************/
class substitution : public variable_name
{
public:
	/// Default constructor
	substitution() noexcept = default;
	/// Constructor
	/// @param n Name and alias
	explicit substitution(const std::stringcase& n)
		: variable_name (n) {}
	/// Constructor
	/// @param s Name and alias
	explicit substitution(const char* s) : 
		substitution{ std::move(std::stringcase(s)) } {}
	/// Constructor
	/// @param n Name
	/// @param a Alias
	substitution(const std::stringcase& n, const std::stringcase& a)
		: variable_name(n, a) {}

	/// Is there a valid OPC list?
	bool has_valid_opc() const noexcept { return valid_opc; }
	/// Sets valid flag for OPC list
	void set_opc_valid (bool valid = true) noexcept { valid_opc = valid; }
	/// Get OPC list
	const ParseUtil::opc_list& get_opc() const noexcept { return opc; }
	/// Get OPC list
	ParseUtil::opc_list& get_opc() noexcept { return opc; }
	/// Is there a valid OPC list?
	bool get_used() const noexcept { return used_in_substitution; }
	/// Sets valid flag for OPC list
	void set_used(bool uis = true) const noexcept { used_in_substitution = uis; }

protected:
	/// Contains a valid OPC list
	bool			valid_opc = false;
	/// OPC list
	opc_list		opc;
	/// used in substitution
	mutable bool	used_in_substitution = false;
};

/** Enumerated type to describe the substitution processing
	@brief Substitution processing enum
************************************************************************/
enum class process_substitution_enum
{
	/// Standard: ignore if not found in substitution list and not published
	standard,
	/// Include all
	all,
	/// Ignore all that are not found
	ignore
};

/** Class to specify a map of channel substitutions
	@brief Map of channel substitution
************************************************************************/
using substitution_map = std::map<std::stringcase, substitution>;

/** Class to specify a list of channel substitutions
	@brief List of channel substitutions
************************************************************************/
class substitution_list
{
public:
	/// Default constructor
	substitution_list() = default;
	/// Constructor
	explicit substitution_list(process_substitution_enum process) noexcept : processing(process) {}
	/// Constructor
	explicit substitution_list(FILE* inp, 
		process_substitution_enum process = process_substitution_enum::standard) noexcept;

	/// Parse a file
	bool parse(FILE* inp) noexcept;
	/// Parse a memory region
	bool parse(const char* p, int len) noexcept;
	/// Add a replacement name
	void add_substitution(const substitution& subst);
	/// Query for relacement name
	/// Returns true, if a valid name is returned, false otherwise
	/// @param name Lookup name, will be replaced if found
	/// @param published This variable is published by default
	/// @param subst Pointer to substitution, if found (null otherwise)
	bool query_substitution(std::stringcase& name, bool published, const substitution** subst = nullptr) const;

	/// Get substitution list
	const substitution_map& get_substitution_list() const noexcept { return list; }
	/// Set substitution list
	void set_substitution_list(const substitution_map& l) { list = l; }

	/// Get ignore
	bool get_ignore() const noexcept { return ignore; }
	/// Set ignore
	void set_ignore(bool ignr) noexcept { ignore = ignr; }
	/// Get ignore_notfound
	process_substitution_enum get_processing() const noexcept { return processing; }
	/// Set ignore_notfound
	void set_processing(process_substitution_enum process) noexcept {	processing = process; }
	/// Get try_no_array_index
	bool get_try_no_array_index() const noexcept { return try_no_array_index; }
	/// Set try_no_array_index
	void set_try_no_array_index(bool nai) noexcept { try_no_array_index = nai; }

	void check_unused_subsititions() const noexcept;
protected:
	/// Replacement table
	substitution_map		list;
	/// Ignore list all together
	bool					ignore = false;
	/// Ignore if not found, otherwise use same name
	process_substitution_enum		processing = process_substitution_enum::standard;
	/// Try lookup with no array indides when direct name look  up fails
	bool					try_no_array_index = true;

	/// called after finish parsing
	void parse_finish() noexcept;
};


/** @} */

}
