// This is the implementation of ParseTpy.
#include "ParseUtil.h"
#include "ParseUtilConst.h"

using namespace std;

/** @file ParseUtil.cpp
	Utility methods to parsing.
 ************************************************************************/
namespace ParseUtil {	

/* optarg::parse
 ************************************************************************/
int optarg::parse (const std::stringcase& arg)
{
	vector<stringcase> list;
	split_string (list, stringcase ("optarg ") + arg, 
		[] (char c)->bool { return isspace (c) != 0; }, true);
	setup (list.size());
	myargc = list.size();
	if (list.empty()) return 0;
	for (size_t i = 0; i < list.size(); ++i) {
		int len = list[i].size() + 4;
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
	myargp = new (std::nothrow) bool [size];
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
	 if (o.opc != no_change) {
		 opc = o.opc;
	 }
	 for (property_map::const_iterator i = o.opc_prop.begin();
		 i != o.opc_prop.end(); ++i) {
			 opc_prop[i->first] = i->second;
	 }
 }
 
/* OPC list members: is_published
 ************************************************************************/
 bool opc_list::is_published () const
 {
	 return (opc == publish);
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
	int num = sscanf_s (s.c_str(), "%i/%i:%i%n", ig, io, sz, &n);
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
	case pt_int :
		return "int";
	case pt_real:
		return "real";
	case pt_bool:
		return "bool";
	case pt_string:
		return "string";
	case pt_enum:
		return "enum";
	case pt_binary:
		return "binary";
	case pt_invalid:
	default:
		return "invalid";
	}
}

/* process_arg::get
 ************************************************************************/
std::stringcase process_arg::get_full () const
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
			set_process_tags (process_all);
			++num;
		}
		// Call process for simple types only
		else if (arg == "-ps" || arg == "/ps") {
			set_process_tags (process_atomic);
			++num;
		}
		// Call process for complex types only
		else if (arg == "-pc" || arg == "/pc") {
			set_process_tags (process_structured);
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