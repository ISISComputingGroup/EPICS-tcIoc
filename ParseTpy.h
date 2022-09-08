#pragma once
#include "stdafx.h"
#include "ParseUtil.h"

/** @file ParseTpy.h
	Header which includes classes to parse a TwinCAT tpy file. 
	It includes "ParseUtil.h" and "ParseTpyTemplate.h"
 ************************************************************************/

/** @namespace ParseTpy
	ParseTpy name space 
 	@brief Namespace for parsing
************************************************************************/
namespace ParseTpy {

/** @defgroup parsetpyopc TwinCAT tpy file parser
 ************************************************************************/
/** @{ */

/** This is a base class for storing the ADS routing information
    @brief ADS routing information
 ************************************************************************/
class ads_routing_info {
public:
	/// Default constructor
	ads_routing_info() noexcept = default;
	/// Constructor
	explicit ads_routing_info (const std::stringcase& netid, int port = 801) noexcept
		: ads_netid (netid), ads_port (port) {}
	/// Constructor
	explicit ads_routing_info (const std::stringcase& netid, int port,
							   const std::stringcase& targetname) noexcept
		: ads_netid (netid), ads_port (port), ads_targetname(targetname) {}

	/// Get ADS net id
	const std::stringcase& get_netid() const noexcept { return ads_netid; }
	/// Set ADS net id
	void set_netid (const std::stringcase& netid) noexcept { ads_netid = netid; }
	/// Get ADS port
	int get_port() const noexcept { return ads_port; }
	/// Set ADS port
	void set_port (int port) noexcept { ads_port = port; }
	/// Get ADS target name
	const std::stringcase& get_targetname() const noexcept { return ads_targetname; }
	/// Set ADS target name
	void set_targetname (const std::stringcase& targetname) { ads_targetname = targetname; }

	/// Checks, if net id is of the form n.n.n.n.n.n
	bool isValid() const noexcept;
	/// Gets a string representation of a ads routing information
	/// @return string with format "tc://netid:port/", empty on error
	std::stringcase get() const;
	/// Set the ads routing information using a string of the form:
	/// "tc://netid:port/" where netid is a string of the format n.n.n.n.n.n
	/// @param s String describing the ads routing information
	/// @return True if successful
	bool set (const std::stringcase& s);
	/// Get address in net format
	/// @param a1 First address qualifier (return)
	/// @param a2 Second address qualifier (return)
	/// @param a3 Third address qualifier (return)
	/// @param a4 Fourth address qualifier (return)
	/// @param a5 Fifth address qualifier (return)
	/// @param a6 Sixth address qualifier (return)
	bool get (unsigned char& a1, unsigned char& a2, unsigned char& a3, 
		unsigned char& a4, unsigned char& a5, unsigned char& a6) const noexcept;

protected:
	/// ADS net ID
	std::stringcase	ads_netid;
	/// ADS port
	int				ads_port = 0;
	/// ADS target name
	std::stringcase	ads_targetname;
};

/** This is a base class for storing the compiler information
	@brief Compiler information
************************************************************************/
class compiler_info {
public:
	/// Default constructor
	compiler_info() noexcept = default;

	/// Get compiler version string
	const std::stringcase& get_cmpl_versionstr() const noexcept { return cmpl_versionstr; }
	/// Set compiler version string
	void set_cmpl_versionstr (const std::stringcase& versionstr);
	/// Get compiler version
	double get_cmpl_version() const noexcept { return cmpl_version; }

	/// Get twincat version string
	const std::stringcase& get_tcat_versionstr() const noexcept { return tcat_versionstr; }
	/// Set twincat version string
	void set_tcat_versionstr (const std::stringcase& versionstr);
	/// Get twincat major version
	unsigned int get_tcat_version_major() const noexcept { return tcat_version_major; }
	/// Get twincat minor version
	unsigned int get_tcat_version_minor() const noexcept { return tcat_version_minor; }
	/// Get twincat build version
	unsigned int get_tcat_version_build() const noexcept { return tcat_version_build; }

	/// Get cpu familiy string
	const std::stringcase& get_cpu_family() const noexcept { return cpu_family; }
	/// Set cpu familiy string
	void set_cpu_family (const std::stringcase& family) {cpu_family = family; };

	/// Checks, if version is of the form n.n...
	bool is_cmpl_Valid() const noexcept;
	/// Checks, if twincat version is of the form n.n...
	bool is_tcat_Valid() const noexcept;

protected:
	/// version string
	std::stringcase	cmpl_versionstr;
	/// version number
	double			cmpl_version = 0.0;
	/// twincat version string
	std::stringcase	tcat_versionstr;
	/// twincat major version number
	unsigned int	tcat_version_major = 0;
	/// twincat minor version number
	unsigned int	tcat_version_minor = 0;
	/// twincat build version number
	unsigned int	tcat_version_build = 0;
	/// cpu family string
	std::stringcase	cpu_family;
};

/* This is a base class for storing the task information
************************************************************************/

/** This is a base class for storing the project information
	@brief Project information
************************************************************************/
class project_record : public ads_routing_info, public compiler_info {
public:
	/// Default constructor
	project_record() = default;

protected:
};


/** This is a base class for storing name, type, type id and opc list
     @brief Base record definition
************************************************************************/
class base_record 
{
public:
	/// Default constructor
	base_record() noexcept = default;
	/// Constructor
	/// @param n Name
	explicit base_record (const std::stringcase& n) : name (n) {}
	/// Constructor
	/// @param n Name
	/// @param o OPC list
	base_record (const std::stringcase& n, const ParseUtil::opc_list& o) 
		: name (n), opc(o) {}
	/// Constructor
	/// @param n Name
	/// @param o OPC list
	/// @param tn Type name
	/// @param td Type decortation or id
	base_record (const std::stringcase& n, const ParseUtil::opc_list& o,
		const std::stringcase& tn, unsigned int td = 0) 
		: name (n), type_n (tn), type_decoration (td), opc(o) {}
	/// Constructor
	/// @param n Name
	/// @param tn Type name
	/// @param td Type decortation or id
	base_record (const std::stringcase& n, 
		const std::stringcase& tn, unsigned int td = 0)
		: name (n), type_n (tn), type_decoration (td) {}

	/// Get name
	const std::stringcase& get_name() const noexcept { return name; }
	/// Get name
	std::stringcase& get_name() noexcept { return name; }
	/// Set name
	void set_name (std::stringcase n) { name = n; }
	/// Get type name 
	const std::stringcase& get_type_name() const noexcept { return type_n; }
	/// Get type name 
	std::stringcase& get_type_name() noexcept { return type_n; }
	/// Set type name
	void set_type_name (std::stringcase t) noexcept { type_n = t; }
	/// Get type decoration
	unsigned int get_type_decoration () const noexcept { return type_decoration; }
	/// Set type decoration 
	void set_type_decoration (unsigned int id) noexcept {type_decoration = id; }
	/// Get type pointer
	bool get_type_pointer () const noexcept { return type_pointer; }
	/// Set type pointer 
	void set_type_pointer (bool isPointer) noexcept {type_pointer = isPointer; }

	/// Get OPC list
	const ParseUtil::opc_list& get_opc() const noexcept { return opc; }
	/// Get OPC list
	ParseUtil::opc_list& get_opc() noexcept { return opc; }

protected:
	/// name of type
	std::stringcase		name;
	/// type definition
	std::stringcase		type_n;
	/// decoration or type ID of type definition
	unsigned int		type_decoration = 0;
	/// this is a pointer
	bool				type_pointer = false;

	/// list of opc properties
	ParseUtil::opc_list	opc;
};

/** This class stores a lbound, elements pair.
************************************************************************/
using dimension = std::pair<int, int>;

/** This list stores lbound, elements pairs.
************************************************************************/
using dimensions = std::list<dimension>;

/** This map stores a list of enum values.
************************************************************************/
using enum_map = std::map<int, std::stringcase>;

/** This type stores an enum pair.
************************************************************************/
using enum_pair = std::pair<int, std::stringcase>;

/** This class stores typed items.
	@brief item record
************************************************************************/
class item_record : public base_record, public ParseUtil::bit_location
{
public:
	/// Default constructor
	item_record() noexcept = default;
};

/** This class stores a list of subitems.
************************************************************************/
using item_list = std::list<item_record>;


/** This structure describes a type record
     @brief Type enum
************************************************************************/
enum class type_enum 
{
	/// Unknown type
	unknown, 
	/// Simple type
	simple, 
	/// Array type
	arraytype, 
	/// Enumerated type
	enumtype, 
	/// Structure type
	structtype,
	/// Function block
	functionblock
};

/** This structure holds a type record
    @brief Type record information
************************************************************************/
class type_record : public base_record, public ParseUtil::bit_location
{
public:
	/// Default constructor
	type_record() noexcept = default;

	/// get the data type
	type_enum get_type_description() const noexcept { return type_desc; }
	/// Set decoration (type ID)
	void set_type_description (type_enum desc) noexcept {type_desc = desc; }
	/// Get decoration (type ID)
	unsigned int get_name_decoration () const noexcept { return name_decoration; }
	/// Set decoration (type ID)
	void set_name_decoration (unsigned int id) noexcept {name_decoration = id; }

	/// Get array dimensions
	const dimensions& get_array_dimensions() const noexcept { return array_list; }
	/// Get array dimensions
	dimensions& get_array_dimensions() noexcept { return array_list; }
	/// Get enumerated list
	const enum_map& get_enum_list() const noexcept { return enum_list; }
	/// Get enumerated list
	enum_map& get_enum_list() noexcept { return enum_list; }
	/// Get structure list
	const item_list& get_struct_list() const noexcept { return struct_subitems; }
	/// Get structure list
	item_list& get_struct_list() noexcept { return struct_subitems; }

protected:
	/// Type description
	type_enum		type_desc = type_enum::unknown;
	/// decoration (type ID) of type name
	unsigned int	name_decoration = 0;

	/// table of dimensions
	dimensions		array_list;
	/// map of enum id and name
	enum_map		enum_list;
	/// list of structure elements
	item_list		struct_subitems;
};

/** This is a multimap to store type records
************************************************************************/
using type_multipmap = std::multimap<unsigned int, type_record>;

/** This is a map of type records, index is type number as defined in tpy
	@brief Type dictionary
************************************************************************/
class type_map : protected type_multipmap
{
public:
	/// value type
	using type_multipmap::value_type;
	using type_multipmap::insert;

	/// Constructor
	type_map() = default;
	/// find an element
	const value_type::second_type* 
	find (value_type::first_type id, const std::stringcase& typn) const;
	// patch type name decorators that are zero but shouldn't
	int patch_type_decorators();
};


/** This structure holds a symbol record
	@brief Symbol record
************************************************************************/
class symbol_record : public base_record, public ParseUtil::memory_location
{
public:
	/// Default constructor
	symbol_record() = default;

	/// get memory location 
	const memory_location& get_location() const noexcept { return *this; }
	/// get memory location 
	memory_location& get_location() noexcept { return *this; }
};

/** This is a list of symbol records
************************************************************************/
using symbol_list = std::list<symbol_record>;


/** This class holds the structure of a tpy file
	@brief Tpy file parsing
************************************************************************/
class tpy_file : public ParseUtil::tag_processing
{
public:
	/// Default constructor
	tpy_file() = default;
	/// Constructor
	tpy_file (FILE* inp);

	/// Parse a file
	bool parse (FILE* inp);
	/// Parse a memory region
	bool parse (const char* p, int len);

	/// Return list of symbols
	const symbol_list& get_symbols() const noexcept { return sym_list; }
	/// Return list of types
	const type_map& get_types() const noexcept { return type_list; }
	/// Return project information
	const project_record& get_project_info() const noexcept { return project_info; }

	/** Iterates over the symbol list and processes all specified tags.
	@param process Function class
	@param prefix Prefix which is added to all variable names
	@return Number of processes variables
	@brief Process the type tree of a symbol
	*/
	template <class Function>
	int process_symbols (Function& process, 
		const std::stringcase& prefix = std::stringcase()) const;

	/** Starts with a symbol and resolves the type information 
	recursevly until an atomic type (like INT) is found. Then, calls
	the process function with an argument of type process_arg.
	The function must return true if suffessful and false otherwise.
	@param symbol Symbol to resolve
	@param process Function class
	@param prefix Prefix which is added to all variable names
	@return Number of processes variables
	@brief Process the type tree of a symbol
	*/
	template <class Function>
	int process_type_tree (const symbol_record& symbol, 
		Function& process, const std::stringcase& prefix = std::stringcase()) const;

	/** Starts with a type and resolves the type information 
	recursevly until an atomic type (like BOOL) is found. Then, calls
	the process function with an argument of type process_arg.
	@param typ Type to resolve
	@param defopc Default list of OPC parameters
	@param loc Memory location of variable
	@param process Function class
	@param varname Name of variable of the specified type (default is "")
	@param level Recursive level (stops when reaching 100, default 0)
	@return Number of processes variables
	@brief Process the type tree of a type
	*/
	template <class Function>
	int process_type_tree (const type_record& typ, 
		ParseUtil::opc_list defopc, const ParseUtil::memory_location& loc, 
		Function& process, 
		const ParseUtil::variable_name& varname = ParseUtil::variable_name(), 
		int level = 0) const;

	/** Starts with a type and resolves the type information 
	recursevly until an atomic type (like STRING) is found. Then, calls
	the process function with an argument of type process_arg.
	@param typ Name of type to resolve
	@param id Decoration or unique ID of type
	@param defopc Default list of OPC parameters
	@param loc Memory location of variable
	@param process Function class
	@param varname Name of variable of the specified type (default is "")
	@param level Recursive level (stops when reaching 100, default 0)
	@return Number of processes variables
	@brief Process the type tree of a type
	*/
	template <class Function>
	int process_type_tree (const std::stringcase& typ, unsigned int id, 
		const ParseUtil::opc_list& defopc, const ParseUtil::memory_location& loc, 
		Function& process, 
		const ParseUtil::variable_name& varname = ParseUtil::variable_name(), 
		int level = 0) const;

protected:
	/// Project information
	project_record	project_info;
	/// List of symbols
	symbol_list		sym_list;
	/// List of types
	type_map		type_list;

	/** This function is called at the end of parsing.
	Here we set the TC server name in the OPC variables for each symbol
	@brief finish up the parsing
	*/
	void parse_finish();

	/** Resolves the type information for an array. Calls the process 
	function for each index with an argument of type process_arg.
	@param typ Name of type to resolve
	@param dim Dimensions of the array
	@param defopc Default list of OPC parameters
	@param loc Memory location of variable
	@param process Function class
	@param varname Name of variable of the specified type (default is "")
	@param level Recursive level (stops when reaching 100, default 0)
	@return Number of processes variables
	@brief Process the type tree of a type
	*/
	template <class Function>
	int process_array (const ParseTpy::type_record& typ, ParseTpy::dimensions dim,
		const ParseUtil::opc_list& defopc, const ParseUtil::memory_location& loc, 
		Function& process, const ParseUtil::variable_name& varname, int level) const;
};

/** @} */

}

/* Include template declaration
 */
#include "ParseTpyTemplate.h"
