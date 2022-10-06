// This is the implementation of ParseTpy.
#include "ParseUtil.h"
#include "ParseUtilConst.h"
#include "ParseTpyConst.h"
#define XML_STATIC ///< Static linking
#include "Expat/expat.h"
#if defined(__amigaos__) && defined(__USE_INLINE__)
#include <proto/expat.h>
#endif

#ifdef XML_LARGE_SIZE
#if defined(XML_USE_MSC_EXTENSIONS) && _MSC_VER < 1400
#define XML_FMT_INT_MOD "I64" ///< Int format
#else
#define XML_FMT_INT_MOD "ll" ///< Int format
#endif
#else
#define XML_FMT_INT_MOD "l" ///< Int format
#endif

using namespace std;
using namespace ParseTpy;


/** @file ParseUtil.cpp
	Utility methods to parsing.
 ************************************************************************/
namespace ParseUtil {	

	/* Forward declaration of XML callbacks
	 ************************************************************************/
	static void XMLCALL startElement(void* userData, const char* name,
		const char** atts);
	static void XMLCALL endElement(void* userData, const char* name);
	static void XMLCALL startCData(void* userData) noexcept;
	static void XMLCALL endCData(void* userData) noexcept;
	static void XMLCALL dataElement(void* userData, const char* data, int len);


	/** This structure keeps track of the parser information.
		@brief Parser information
	 ************************************************************************/
	class parserinfo_type
	{
	public:
		/// Constructor
		parserinfo_type(substitution_list& l) noexcept
			: ignore(0), tag(0), cdata(0), opc_parse(0), opc_cdata(0), list(&l) {}

		/// Initialze temporary parser info
		void init();

		/// ignore elements during parsing (with level)
		int				ignore;
		/// parsing withing a tag
		/// level indicators in general: 
		///		0 - not encountered, 
		///		1 - within tag,
		///		2 - within tc
		///		3 - within epics
		///		4 - within properties
		///		5 - within property
		int				tag;
		/// CDATA indicator
		int				cdata;
		/// Substitution
		substitution	subst;
		/// temporary opc element
		property_el		opc_prop;
		/// level indicator for opc element
		int				opc_parse;
		/// temporary data string for parsed opc data
		std::stringcase	opc_data;
		/// temporary cdata indicator
		int				opc_cdata;

		/// Add substitution
		void add_substitution() {
			if (list) list->add_substitution(subst);
		}
	protected:
		/// pointer to symbol list
		substitution_list* list;
	private:
		/// Default constructor
		parserinfo_type() = delete;
	};



/* replacement_rules::parse_rules
 ************************************************************************/
bool replacement_rules::parse_rules(const std::stringcase& s, const std::stringcase alias)
{
	if (!alias.empty()) table[OPC_NAME_ALIAS] = alias;
	std::regex e("([^=,]+)=([^=,]*)");
	std::cmatch m;
	const char* p = s.c_str();
	while (std::regex_search(p, m, e)) {
		if (m.size() == 3) {
			std::stringcase var(m[1].str().c_str());
			trim_space(var);
			std::stringcase val(m[2].str().c_str());
			trim_space(val);
			if (!var.empty()) add_rule(var, val);
		}
		p += m.length();
	}
	return !*p;
}

/* replacement_rules::get_rule_string
 ************************************************************************/
std::stringcase replacement_rules::get_rule_string() const
{
	stringcase msg;
	for (const auto& i : table) {
		msg += i.first + "=" + i.second + ",";
	}
	if (!msg.empty()) {
		msg.erase(msg.length() - 1, std::stringcase::npos);
	}
	return msg;
}

/* replacement_rules::apply_replacement_rules
 ************************************************************************/
std::stringcase replacement_rules::apply_replacement_rules(const std::stringcase& arg) const
{
	stringcase ret(arg);
	stringcase var;
	stringcase val;
	stringcase::size_type pos1 = 0;
	stringcase::size_type pos2 = 0;
	const stringcase::size_type prefixlen = strlen(prefix);
	const stringcase::size_type suffixlen = strlen(suffix);
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
		ret.replace(pos1, pos2 - pos1 + suffixlen, var);
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
		[] (char c) noexcept -> bool { return isspace (c) != 0; }, true);
	const size_t size = list.size();
	setup (size);
	myargc = (int)size;
	if (list.empty()) return 0;
	for (size_t i = 0; i < size; ++i) {
		const size_t len = list[i].size();
		myargv[i] = new (std::nothrow) char[len + 4];
		if (myargv[i]) strncpy_s(myargv[i], len + 4, list[i].c_str(), len);
	}
	return myargc;
}

/* optarg::all_done
 ************************************************************************/
bool optarg::all_done() const noexcept
{
	if (!myargp) return true;
	for (int i = 0; i < myargc; ++i) {
		if (!myargp[i]) return false;
	}
	return true;
}

/* optarg::setup
 ************************************************************************/
void optarg::setup (size_t size) noexcept
{
	// deallocate
	for (size_t i = 0; i < mysize; ++i) {
		delete[] myargv[i];
	}
	try {
		if (size == 0) {
			myargv = nullptr;
			myargp = nullptr;
		}
		else {
			myargv = make_unique<char* []>(size);
			myargp = make_unique<bool[]>(size);
			for (size_t i = 0; i < size; ++i) {
				myargv[i] = nullptr;
				myargp[i] = false;
			}
		}
		mysize = size;
	}
	catch (...) {
		mysize = 0;
	}
	myargc = 0;
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
 bool opc_list::is_published () const noexcept
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
 bool memory_location::isValid() const noexcept
 { 
	 return (igroup >= 0) && (ioffset >= 0) && (bytesize > 0); 
 }

/* memory_location::section
 ************************************************************************/
bool memory_location::set_section (const bit_location& loc) noexcept
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
bool memory_location::set (const std::stringcase& s) noexcept
{
	int ig (0), io (0), sz (0), n (0);
	const int num = sscanf_s (s.c_str(), "%i/%i:%i%n", &ig, &io, &sz, &n);
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
		size = strtol(s.c_str(), nullptr, 10);;
	}
	else if (ptype == process_type_enum::pt_int) {
		if (get_type_name() == "SINT" || get_type_name() == "USINT" || get_type_name() == "BYTE")
			size = 1;
		else if (get_type_name() == "INT" || get_type_name() == "UINT" || get_type_name() == "WORD")
			size = 2;
		else if (get_type_name() == "DINT" || get_type_name() == "UDINT" || get_type_name() == "DWORD" || 
			     get_type_name() == "TIME" || get_type_name() == "TOD"   || get_type_name() == "DATE"  || 
			     get_type_name() == "DT" || get_type_name() == "TIME_OF_DAY" || get_type_name() == "DATE_AND_TIME")
			size = 4;
		else if (get_type_name() == "LINT" || get_type_name() == "ULINT" || get_type_name() == "LWORD" || 
			     get_type_name() == "LTIME")
			size = 8;
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
	std::stringcase n = memloc.get();
	if (!n.empty()) {
		return servername + n;
	}
	else {
		return "";
	}
}


/* tag_processing::getopt
 ************************************************************************/
int tag_processing::getopt (int argc, const char* const argv[], bool argp[]) noexcept
{
	int num = 0;
	for (int i = 1; i < argc; ++i) {
		if (argp && argp[i]) continue;
		if (!argv[i]) {
			if (argp) argp[i] = true;
			continue;
		}
		std::stringcase arg (argv[i]);
		const int oldnum = num;
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


/* substitution_list::substitution_list
 ************************************************************************/
substitution_list::substitution_list(FILE* inp, process_substitution_enum process) noexcept
	: processing(process)
{
	parse(inp);
}

/* substitution_list::parse
 ************************************************************************/
bool substitution_list::parse(FILE* inp) noexcept
{
	if (!inp) {
		return false;
	}

	// Set up parser info
	parserinfo_type info(*this);

	// Initialize XML parser
	char buf[BUFSIZ] = "";
	XML_Parser parser = XML_ParserCreate(NULL);
	XML_SetUserData(parser, &info);
	XML_SetElementHandler(parser, startElement, endElement);
	XML_SetCdataSectionHandler(parser, startCData, endCData);
	XML_SetCharacterDataHandler(parser, dataElement);

	// read data and parse
	bool done = false;
	do {
		const int len = (int)fread(buf, 1, sizeof(buf), inp);
		done = len < sizeof(buf);
#pragma warning (disable:26812)
		if (XML_Parse(parser, buf, len, done) == XML_Status::XML_STATUS_ERROR) {
			fprintf(stderr,
				"%s at line %" XML_FMT_INT_MOD "u\n",
				XML_ErrorString(XML_GetErrorCode(parser)),
				XML_GetCurrentLineNumber(parser));
			return false;
		}
#pragma warning (default:26812)
	} while (!done);

	// Finish up
	XML_ParserFree(parser);
	parse_finish();
	return true;
}

/* substitution_list::parse
 ************************************************************************/
bool substitution_list::parse(const char* p, int len) noexcept
{
	if (!p || (len < 0)) {
		return false;
	}
	// Set up parser info
	parserinfo_type info(*this);

	// Initialize XML parser
	XML_Parser parser = XML_ParserCreate(NULL);
	XML_SetUserData(parser, &info);
	XML_SetElementHandler(parser, startElement, endElement);
	XML_SetCdataSectionHandler(parser, startCData, endCData);
	XML_SetCharacterDataHandler(parser, dataElement);

	// read data and parse
	if (XML_Parse(parser, p, len, true) == XML_Status::XML_STATUS_ERROR) {
		fprintf(stderr,
			"%s at line %" XML_FMT_INT_MOD "u\n",
			XML_ErrorString(XML_GetErrorCode(parser)),
			XML_GetCurrentLineNumber(parser));
		return false;
	}

	// Finish up
	XML_ParserFree(parser);
	parse_finish();
	return true;
}

/* substitution_list::add_substitution
 ************************************************************************/
void substitution_list::add_substitution(const substitution& subst)
{
	if (!subst.get_name().empty()) list[subst.get_name()] = subst;
}

/* substitution_list::query_substitution
 ************************************************************************/
bool substitution_list::query_substitution(std::stringcase& name,
	bool published, const substitution** subst) const
{
	if (subst != nullptr) *subst = nullptr;
	if (ignore) return true;
	if (list.empty()) {
		return (get_processing() == process_substitution_enum::all) ||
			   ((get_processing() == process_substitution_enum::standard) && published);
	}

	// try finding name in list
	auto iter = list.find(name);
	if (iter != list.end()) {
		name = iter->second.get_alias();
		if (subst != nullptr) *subst = &(iter->second);
		iter->second.set_used();
		return true;
	}
	// try again with array brackets replaced
	if (try_no_array_index) {
		std::stringcase n(name);
		stringcase::size_type pos = 0;
		while ((pos = n.find('[')) != stringcase::npos) n[pos] = '_';
		while ((pos = n.find(']')) != stringcase::npos) n.erase(pos, 1);
		iter = list.find(n);
		if (iter != list.end()) {
			name = iter->second.get_alias();
			if (subst != nullptr) *subst = &(iter->second);
			iter->second.set_used();
			return true;
		}
	}
	return (get_processing() == process_substitution_enum::all) ||
		   ((get_processing() == process_substitution_enum::standard) && published);
}

/* substitution_list::check_unused_subsititions
 ************************************************************************/
void substitution_list::check_unused_subsititions() const noexcept
{
	if (ignore || (get_processing() == process_substitution_enum::all)) return;
	for (const auto& iter : list) {
		const substitution& subst = iter.second;
		if (!subst.get_used()) {
			printf("Substitution name %s unused.\n", subst.get_name().c_str());
		}
	}
}

/* substitution_list::parse_finish
 ************************************************************************/
void substitution_list::parse_finish() noexcept
{
	// look for PROP[8620]: Alias 
	//std::stringcase alias;
	//for (auto& iter : list) {
	//	substitution& subst = iter.second;
	//	if (subst.get_alias().empty() && subst.has_valid_opc() &&
	//		subst.get_opc().get_property(OPC_PROP_ALIAS, alias) && !alias.empty()) {
	//		subst.get_alias() = alias;
	//	}
	//}
}

/************************************************************************/
/* XML Parsing
 ************************************************************************/

/* XML start element function callback
 ************************************************************************/
static void XMLCALL startElement(void* userData, const char* name,
	const char** atts)
{
	parserinfo_type* pinfo = (parserinfo_type*)userData;
	if (pinfo->ignore) {
		++pinfo->ignore;
		return;
	}

	std::stringcase n(name ? name : "");

	// Substitution
	if ((pinfo->tag == 0) && (n.compare(xmlSubstitution) == 0)) {
		pinfo->tag = 0;
	}
	// Symbol
	else if ((pinfo->tag == 0) && (n.compare(xmlSymbol) == 0)) {
		pinfo->subst = substitution();
		pinfo->subst.get_opc().set_opc_state(opc_enum::publish);
		pinfo->tag = 1;
	}
	// Name of variable
	else if ((pinfo->tag == 1) && (n.compare(xmlName) == 0)) {
		pinfo->subst.get_name().clear();
		pinfo->tag = 2;
	}
	// Alias name
	else if ((pinfo->tag == 1) && (n.compare(xmlAlias) == 0)) {
		pinfo->subst.get_alias().clear();
		pinfo->tag = 3;
	}
	// opc properties
	else if ((pinfo->tag == 1) && (n.compare(xmlProperties) == 0)) {
		pinfo->subst.set_opc_valid();
		pinfo->tag = 4;
		pinfo->opc_parse = 1;
	}
	// opc property
	else if (pinfo->opc_parse == 1 && n.compare(xmlProperty) == 0) {
		pinfo->opc_parse = 2;
		pinfo->opc_prop = property_el();
	}
	// opc property name
	else if (pinfo->opc_parse == 2 && n.compare(xmlName) == 0) {
		pinfo->opc_parse = 3;
		pinfo->opc_data = "";
		pinfo->opc_cdata = 1;
	}
	// opc property value
	else if (pinfo->opc_parse == 2 && n.compare(xmlValue) == 0) {
		pinfo->opc_parse = 4;
		pinfo->opc_data = "";
		pinfo->opc_cdata = 1;
	}
	// ignore all tags surrounding Symbol
	else if (pinfo->opc_parse == 0) {
		// nothing
	}
	else {
		++pinfo->ignore;
	}
}

/* XML end element function callback
 ************************************************************************/
static void XMLCALL endElement(void* userData, const char* name)
{
	parserinfo_type* pinfo = (parserinfo_type*)userData;
	if (pinfo->ignore) {
		--pinfo->ignore;
		return;
	}
	std::stringcase n(name ? name : "");

	// Substitution
	if ((pinfo->tag == 0) && (n.compare(xmlSubstitution) == 0)) {
		pinfo->tag = 0;
	}
	// parsing tag information
	else if ((pinfo->tag == 1) && (n.compare(xmlSymbol) == 0)) {
		// look for PROP[8620]: Alias 
		std::stringcase alias;
		if (pinfo->subst.get_alias().empty() && pinfo->subst.has_valid_opc() &&
			pinfo->subst.get_opc().get_property(OPC_PROP_ALIAS, alias) && !alias.empty()) {
			pinfo->subst.get_alias() = alias;
		}
		// no check we have a valid substitution
		if (!pinfo->subst.get_name().empty() && 
			!pinfo->subst.get_alias().empty()) {
			pinfo->add_substitution();
		}
		pinfo->tag = 0;
	}
	// TwinCAT tag name
	else if ((pinfo->tag == 2) && (n.compare(xmlName) == 0)) {
		trim_space(pinfo->subst.get_name());
		pinfo->tag = 1;
	}
	// EPICS channel name
	else if ((pinfo->tag == 3) && (n.compare(xmlAlias) == 0)) {
		trim_space(pinfo->subst.get_alias());
		pinfo->tag = 1;
	}
	// opc properties
	else if (pinfo->opc_parse == 1 && n.compare(xmlProperties) == 0) {
		pinfo->opc_parse = 0;
		pinfo->tag = 1;
	}
	// opc property
	else if (pinfo->opc_parse == 2 && n.compare(xmlProperty) == 0) {
		pinfo->opc_parse = 1;
		if (pinfo->opc_prop.first == -1) {
			const int num = strtol(pinfo->opc_prop.second.c_str(), NULL, 10);
			pinfo->subst.get_opc().set_opc_state(num ? opc_enum::publish : opc_enum::silent);
		}
		else if (pinfo->opc_prop.first > 0) {
			pinfo->subst.get_opc().get_properties().insert(pinfo->opc_prop);
			pinfo->subst.set_opc_valid();
		}
	}
	// opc property name
	else if (pinfo->opc_parse == 3 && n.compare(xmlName) == 0) {
		pinfo->opc_parse = 2;
		trim_space(pinfo->opc_data);
		int num = 0;
		if (pinfo->opc_data.compare(opcExport) == 0) num = -1;
		else if (pinfo->opc_data.compare(0, 8, opcProp) == 0) {
			pinfo->opc_data.erase(0, 8);
			trim_space(pinfo->opc_data);
			if (pinfo->opc_data.compare(0, 1, opcBracket) == 0)
				pinfo->opc_data.erase(0, 1);
			num = strtol(pinfo->opc_data.c_str(), NULL, 10);
		}
		pinfo->opc_prop.first = num;
	}
	// opc property value
	else if (pinfo->opc_parse == 4 && n.compare(xmlValue) == 0) {
		pinfo->opc_parse = 2;
		pinfo->opc_prop.second = pinfo->opc_data;
	}
}

/* XML start CData function callback
 ************************************************************************/
static void XMLCALL startCData(void* userData) noexcept
{
	parserinfo_type* pinfo = (parserinfo_type*)userData;
	if (pinfo->ignore) {
		return;
	}
	if (pinfo->tag == 4) {
		// clear opc data
		if (pinfo->opc_parse >= 3) {
			pinfo->opc_data.clear();
			pinfo->opc_cdata = 1;
		}
	}
}

/* XML end CData function callback
 ************************************************************************/
static void XMLCALL endCData(void* userData) noexcept
{
	parserinfo_type* pinfo = (parserinfo_type*)userData;
	if (pinfo->ignore) {
		return;
	}
	if (pinfo->tag == 4) {
		// clear opc data
		if (pinfo->opc_parse >= 3) {
			pinfo->opc_cdata = 0;
		}
	}
}


/* XML data function callback
 ************************************************************************/
static void XMLCALL dataElement(void* userData, const char* data, int len)
{
	parserinfo_type* pinfo = (parserinfo_type*)userData;
	if (pinfo->ignore) {
		return;
	}

	// get data for tc name
	if (pinfo->tag == 2) {
		pinfo->subst.get_name().append(data, len);
	}
	// get data for epics name
	else if (pinfo->tag == 3) {
		pinfo->subst.get_alias().append(data, len);
	}
	// get OPC data
	else if (pinfo->tag == 4) {
		// append opc data
		if (pinfo->opc_parse >= 3) {
			if (pinfo->opc_cdata)
				pinfo->opc_data.append(data, len);
		}
	}
}

}