// This is the implementation of ParseTpy.
#include "ParseUtil.h"
#include "ParseUtilConst.h"

using namespace std;

/** @file ParseUtil.cpp
	Utility methods to parsing.
 ************************************************************************/
namespace ParseUtil {	

/* Static const variables
	************************************************************************/
const char* const replacement_rules::prefix = "${";
const char* const replacement_rules::suffix = "}";

/* Static const variables
	************************************************************************/
std::stringcase replacement_rules::apply_replacement_rules(const std::stringcase& arg) const
{
	stringcase ret(arg);
	stringcase var;
	stringcase val;
	stringcase::size_type pos1;
	stringcase::size_type pos2;
	stringcase::size_type prefixlen = strlen(prefix);
	stringcase::size_type suffixlen = strlen(suffix);
	replacement_table::const_iterator rep;

	// start from beginning
	pos2 = 0;
	while ((pos1 = ret.find(prefix, pos2)) != stringcase::npos) {
		// look for suffix
		pos2 = ret.find(suffix, pos1 + prefixlen);
		// no suffix? What's up? remove prefix and move on
		if (pos2 == stringcase::npos) {
			ret.erase(pos1, prefixlen);
			// if recursive start from beginning
			pos2 = is_recursive() ? 0 : pos1;
			continue;
		}
		// determine variable name
		var = ret.substr(pos1 + prefixlen, pos2 - (pos1 + prefixlen));
		trim_space(var);
		// check for value in table
		if (!var.empty() &&
			(rep = table.find(var)) != table.end()) {
			var = rep->second;
		}
		// replace var with value
		ret.replace(pos1, pos2 - pos1 + 1, var);
		// if recursive start from beginning
		pos2 = is_recursive() ? 0 : pos1 + var.size();
	}
	return ret;
}

/* optarg::parse
 ************************************************************************/
int optarg::parse (const std::stringcase& arg)
{
	vector<stringcase> list;
	split_string (list, stringcase ("optarg ") + arg, 
		[] (char c)->bool { return isspace (c) != 0; }, true);
	setup (static_cast<int>(list.size()));
	myargc = static_cast<int>(list.size());
	if (list.empty()) return 0;
	for (size_t i = 0; i < list.size(); ++i) {
		size_t len = list[i].size() + 4;
		myargv[i] = new (std::nothrow) char[len];
		if (myargv[i]) 
			strncpy_s (myargv[i], len, list[i].c_str(), list[i].size());
	}
	return myargc;
}

/* optarg::all_done
 ************************************************************************/
bool optarg::all_done() const
{
	if (!myargp) return true;
	for (int i = 0; i < myargc; ++i) {
		if (!myargp[i]) return false;
	}
	return true;
}

/* optarg::setup
 ************************************************************************/
void optarg::setup (int size)
{
	// deallocate
	if (myargv) {
		for (int i = 0; i < mysize; ++i) 
			if (myargv[i]) delete [] myargv[i];
		delete [] myargv;
		myargv = nullptr;
	}
	if (myargp) delete [] myargp;
	myargp = nullptr;
	mysize = 0;
	myargc = 0;
	if (size <= 0) return;

	myargv = new (std::nothrow) char* [size];
	if (!myargv) {
		mysize = 0;
		return;
	}
	myargp = new (std::nothrow) bool [size];
	if (!myargp) {
		mysize = 0;
		delete[] myargv;
		myargv = nullptr;
		return;
	}
	for (int i = 0; i < size; ++i) {
		myargv[i] = nullptr;
		myargp[i] = false;
	}
	myargc = 0;
	mysize = size;
}



/* OPC list members: add
 ************************************************************************/
 void opc_list::add (const opc_list& o) 
 {
	 if (o.opc != opc_enum::no_change) {
		 opc = o.opc;
	 }
	 for (const auto& i : o.opc_prop) {
		 opc_prop[i.first] = i.second;
	 }
 }
 
/* OPC list members: is_published
 ************************************************************************/
 bool opc_list::is_published () const
 {
	 return (opc == opc_enum::publish);
 }

/* OPC list members: is_readonly
 ************************************************************************/
 bool opc_list::is_readonly() const
 {
	 bool ro = false; // default

	 // check porperty right and set input or in/out field type
	 int num = 0;
	 if (get_property (OPC_PROP_RIGHTS, num)) {
		 if (num == 1) ro = true;
	 }
	 // check PROP_INOUT for override 
	 std::stringcase s;
	 if (get_property (OPC_PROP_INOUT, s)) {
		 if (s == OPC_PROP_INPUT) ro = true;
		 else if (s == OPC_PROP_OUTPUT) ro = false;
	 }
	 return ro;
 }

/* OPC list members: get_property (string)
 ************************************************************************/
bool opc_list::get_property (int prop, std::stringcase& val) const
{
	property_map::const_iterator i = opc_prop.find (prop);
	if (i == opc_prop.end()) {
		return false;
	}
	val = i->second;
	return true;
}

/* OPC list members: get_property (int)
 ************************************************************************/
bool opc_list::get_property (int prop, int& val) const
{
	std::stringcase sval;
	if (!get_property (prop, sval)) {
		return false;
	}
	char* end = 0;
	int num = strtol (sval.c_str(), &end, 10);
	if (end == sval.c_str()) {
		return false;
	}
	val = num;
	return true;
}

/* OPC list members: get_property (double)
 ************************************************************************/
bool opc_list::get_property (int prop, double& val) const
{
	std::stringcase sval;
	if (!get_property (prop, sval)) {
		return false;
	}
	char* end = 0;
	double num = strtod (sval.c_str(), &end);
	if (end == sval.c_str()) {
		return false;
	}
	val = num;
	return true;
}


/*variable_name::append
 ************************************************************************/
void variable_name::append (const std::stringcase& n, const std::stringcase& sep)
{
	name += sep;
	name += n;
	alias += sep;
	alias += n;
}


/*variable_name::append
 ************************************************************************/
void variable_name::append (const std::stringcase& n, const opc_list& opc, 
		const std::stringcase& sep)
{
	name += sep;
	name += n;
	alias += sep;
	std::stringcase a;
	if (opc.get_property (OPC_PROP_ALIAS, a)) {
		std::trim_space (a);
		alias += a;
	}
	else {
		alias += n;
	}
}


/* memory_location::isValid
 ************************************************************************/
 bool memory_location::isValid() const 
 { 
	 return (igroup >= 0) && (ioffset >= 0) && (bytesize > 0); 
 }

/* memory_location::section
 ************************************************************************/
bool memory_location::set_section (const bit_location& loc)
{
	if (!loc.isValid() || !isValid() ||
		(loc.get_bit_offset() % 8 != 0) || 
		(loc.get_bit_size() % 8 != 0) ||
		(loc.get_bit_offset() + loc.get_bit_size() > 8 * bytesize)) {
		return false;
	}
	ioffset += loc.get_bit_offset() / 8;
	bytesize = loc.get_bit_size() / 8;
	return true;
}

/* memory_location::get
 ************************************************************************/
std::stringcase memory_location::get() const
{
	if ((igroup == -1) || (ioffset == -1) || (bytesize <= 0)) {
		return "";
	}
	char buf[256];
	sprintf_s (buf, sizeof (buf), "%i/%i:%i", igroup, ioffset, bytesize);
	buf[255] = 0;
	return buf;
}

/* memory_location::set
 ************************************************************************/
bool memory_location::set (const std::stringcase& s)
{
	int ig (0), io (0), sz (0), n (0);
	int num = sscanf_s (s.c_str(), "%i/%i:%i%n", &ig, &io, &sz, &n);
	if ((num != 3) || (n != s.length())) {
		igroup = -1;
		ioffset = -1;
		bytesize = -1;
		return false;
	}
	igroup = ig;
	ioffset = io;
	bytesize = sz;
	return true;
}


/* process_arg::get_process_string
 ************************************************************************/
std::stringcase process_arg::get_process_string () const
{
	switch (ptype) {
	case process_type_enum::pt_int :
		return "int";
	case process_type_enum::pt_real:
		return "real";
	case process_type_enum::pt_bool:
		return "bool";
	case process_type_enum::pt_string:
		return "string";
	case process_type_enum::pt_enum:
		return "enum";
	case process_type_enum::pt_binary:
		return "binary";
	case process_type_enum::pt_invalid:
	default:
		return "invalid";
	}
}

/* process_arg_tc::get
 ************************************************************************/
void process_arg::deduce_size()
{
	if (ptype == process_type_enum::pt_string) {
		std::stringcase s(type_n);
		while ((s.length() > 0) && !isdigit(s[0])) s.erase(0, 1);
		int len = strtol(s.c_str(), nullptr, 10);
		size = len;
	}
	else if (ptype == process_type_enum::pt_int) {
		if (get_type_name() == "SINT" || get_type_name() == "USINT" || get_type_name() == "BYTE")
			size = 1;
		else if (get_type_name() == "INT" || get_type_name() == "UINT" || get_type_name() == "WORD")
			size = 2;
		else if (get_type_name() == "DINT" || get_type_name() == "UDINT" || get_type_name() == "DWORD")
			size = 4;
		else if (get_type_name() == "LINT" || get_type_name() == "ULINT" || get_type_name() == "LWORD")
			size = 4;
		else
			size = 0;
	}
	else {
		size = 0;
	}
}

/* process_arg_tc::get
 ************************************************************************/
std::stringcase process_arg_tc::get_full () const
{
	std::stringcase servername = "tc://0.0.0.0.0.0:801/";
	opc.get_property (OPC_PROP_PLCNAME, servername);
	std::stringcase name = memloc.get();
	if (!name.empty()) {
		return servername + name;
	}
	else {
		return "";
	}
}


/* tag_processing::getopt
 ************************************************************************/
int tag_processing::getopt (int argc, const char* const argv[], bool argp[])
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
		// export only opc variables (default)
		if (arg == "-eo" || arg == "/eo" ) {
			set_export_all (false);
			++num;
		}
		// export all
		else if (arg == "-ea" || arg == "/ea" ) {
			set_export_all (true);
			++num;
		}
		// Exclude string types (default)
		else if (arg == "-ns" || arg == "/ns") {
			set_no_strings (true);
			++num;
		}
		// Include string types
		else if (arg == "-ys" || arg == "/ys") {
			set_no_strings (false);
			++num;
		}
		// Call process for all types (default)
		else if (arg == "-pa" || arg == "/pa") {
			set_process_tags (process_tag_enum::all);
			++num;
		}
		// Call process for simple types only
		else if (arg == "-ps" || arg == "/ps") {
			set_process_tags (process_tag_enum::atomic);
			++num;
		}
		// Call process for complex types only
		else if (arg == "-pc" || arg == "/pc") {
			set_process_tags (process_tag_enum::structured);
			++num;
		}
		// no set flag to indicated a processed option
		if (argp && (num > oldnum)) {
			argp[i] = true;
		}
	}
	return num;
}

}