// This is the implementation of ParseTpy.
#include "ParseTpy.h"
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

using namespace ParseUtil;

/** @file ParseTpy.cpp
	Methods to parse TwinCAT tpy file.
 ************************************************************************/

namespace ParseTpy {	

/* Forward declaration of XML callbacks
 ************************************************************************/
static void XMLCALL startElement (void *userData, const char *name, 
	const char **atts);
static void XMLCALL endElement (void *userData, const char *name);
static void XMLCALL startCData (void *userData) noexcept;
static void XMLCALL endCData (void *userData) noexcept;
static void XMLCALL dataElement (void *userData, const char *data, int len);

/** This structure keeps track of the parser information.
	@brief Parser information
 ************************************************************************/
class parserinfo_type 
{
public:
	/// Constructor
	parserinfo_type (project_record& p, symbol_list& s, type_map& t) noexcept
		: ignore (0), projects (false), routing (0), compiler (0), types (0), 
		symbols(0), name_parse (0), type_parse (0), opc_cur (0), 
		opc_parse (0), opc_cdata (0), igroup_parse (0), 
		ioffset_parse (0), bitsize_parse (0), 
		bitoffs_parse (0), array_parse (0), enum_parse (0), 
		struct_parse (0), fb_parse(0),
		sym_list (&s), type_list (&t), project_info(&p) {}

	/// Get symbol list
	symbol_list& get_symbols() noexcept { return *sym_list; }
	/// Get type list
	type_map& get_types() noexcept { return *type_list; }
	/// Get project information
	project_record& get_projectinfo() noexcept { return *project_info; }

	/// Initialze temporary parser info
	void init ();
	/// Get type of parsed object
	type_enum get_type_description() const noexcept;

	/// the very top of the xml tag hierarchy (not within any tag)
	bool verytop() noexcept {
		return !projects && !ignore && !routing && !compiler && !types && !symbols; }
	/// the top of the xml tag hierarchy (within the PlcProjectInfo tag)
	bool top() noexcept {
		return projects && !ignore && !routing && !compiler && !types && !symbols; }

	/// ignore elements during parsing (with level)
	int				ignore;
	/// parsing withing PlcProjectInfo tag
	bool			projects;
	/// parsing within Routing tag (1) or AdsInfo (2), Net ID (3), Port (4), Target name (5)
	int				routing;
	/// parsing within compiler tag (1) or compiler version (2), Twincat version (3), CPU family (4)
	int				compiler;
	/// parsing within DataTypes tag (1) or DataType tag (2)
	int				types;
	/// parsing within Symbols tag (1) or Symbol tag (2)
	int				symbols;

	/// temporary symbol information during parsing
	symbol_record	sym;
	/// temporary type information during parsing
	type_record		rec;

	/// level indicator for type name parsing
	/// level indicators in general: 
	///		0 - not encountered, 
	///		1 - parsed,
	///		2 - currently processing,
	///		3 - further sub tag parsing
	int				name_parse;
	/// level indicator for type parsing
	int				type_parse;
	/// Pointer to current opc list (types only)
	opc_list*		opc_cur;
	/// temporary opc element
	property_el		opc_prop;
	/// level indicator for opc element
	int				opc_parse;
	/// temporary data string for parsed opc data
	std::stringcase	opc_data;
	/// temporary cdata indicator
	int				opc_cdata;
	/// level indicator for IGroup
	int				igroup_parse;
	/// level indicator for IOffset
	int				ioffset_parse;
	/// level indicator for BitSize
	int				bitsize_parse;
	/// level indicator for BitOffs
	int				bitoffs_parse;
	/// temporary data string for parsed igroup/ioffset/bitsize/bitoffs
	std::stringcase	data;

	/// level indicator for array info parsing
	int				array_parse;
	/// temporary data string for parsed array info
	std::stringcase	array_data;
	/// temporary array dimension element
	dimension		array_bounds;
	/// level indicator for enum parsing
	int				enum_parse;
	/// temporary data string for parsed enum data
	std::stringcase	enum_data;
	/// temporary enum element
	enum_pair		enum_element;
	/// temporary enum comment
	std::stringcase	enum_comment;
	/// level indicator for struct parsing
	int				struct_parse;
	/// temporary structure element
	item_record		struct_element;
	/// level indicator for function block parsing
	int				fb_parse;

protected:
	/// pointer to symbol list
	symbol_list*	sym_list;
	/// pointer to type list
	type_map*		type_list;
	/// pointer to project info
	project_record*	project_info;

private:
	/// Default constructor
	parserinfo_type() = delete;
};


/* parserinfo_type::init
 ************************************************************************/
 void parserinfo_type::init() 
 {
	 sym = symbol_record();
	 rec = type_record();
	 name_parse = 0;
	 type_parse = 0;
	 opc_cur = 0;
	 opc_prop = property_el();
	 opc_parse = 0;
	 opc_data = std::stringcase ("");
	 opc_cdata = 0;
	 igroup_parse = 0;
	 ioffset_parse = 0;
	 bitsize_parse = 0;
	 data = std::stringcase ("");
	 array_parse = 0;
	 array_data = std::stringcase ("");
	 array_bounds = dimension (0, 0);
	 enum_parse = 0;
	 enum_data = std::stringcase ("");
	 enum_element = enum_pair (0, std::stringcase (""));
	 struct_parse = 0;
	 struct_element = item_record ();
	 fb_parse = 0;
 }

/* parserinfo_type::get_type_description
 ************************************************************************/
 type_enum parserinfo_type::get_type_description() const noexcept
 {
	 if (name_parse != 1) return type_enum::unknown;
	 if (array_parse == 1 && type_parse == 1) return type_enum::arraytype;
	 if (enum_parse == 1) return type_enum::enumtype;
	 if (struct_parse == 1 && !fb_parse) return type_enum::structtype;
	 if (struct_parse == 1 &&  fb_parse) return type_enum::functionblock;
	 if (type_parse == 1) return type_enum::simple;
	 return type_enum::unknown;
 }


/* ads_routing_info::get
 ************************************************************************/
bool ads_routing_info::isValid() const noexcept
{
	if (ads_netid.empty() || (ads_port < 0)) {
		return false;
	}
	int end = 0;
	const int num = sscanf_s (ads_netid.c_str(), "%*i.%*i.%*i.%*i.%*i.%*i%n", &end);
	if ((num != 0) || (end != ads_netid.length())) {
		return false;
	}
	return true;
}

/* ads_routing_info::get
 ************************************************************************/
std::stringcase ads_routing_info::get() const
{
	if (!isValid()) {
		return "";
	}
	char buf[256];
	sprintf_s (buf, sizeof (buf), "tc://%s:%i/", ads_netid.c_str(), ads_port);
	buf[255] = 0;
	return buf;
}

/* ads_routing_info::get
 ************************************************************************/
bool ads_routing_info::get (unsigned char& a1, unsigned char& a2, 
							unsigned char& a3, unsigned char& a4, 
							unsigned char& a5, unsigned char& a6) const noexcept
{
	if (!isValid()) {
		return false;
	}
	int end = 0;
	int b1(0), b2(0), b3(0), b4(0), b5(0), b6 (0); 
	const int num = sscanf_s (ads_netid.c_str(), "%i.%i.%i.%i.%i.%i%n", 
		&b1, &b2, &b3, &b4, &b5, &b6, &end);
	if ((num == 0) || (num == EOF) || (end != ads_netid.length())) {
		return false;
	}
	a1 = b1; a2 = b2; a3 = b3;
	a4 = b4; a5 = b5; a6 = b6;
	return true;
}

/* ads_routing_info::set
 ************************************************************************/
bool ads_routing_info::set (const std::stringcase& s)
{
	char buf[256] = "";
	int p = 0;
	int end = 0;
	const int num = sscanf_s (s.c_str(), " tc://%255s:%i%n", 
		buf, static_cast<unsigned>(sizeof (buf)), &p, &end);
	if ((num != 2) || (end != s.length())) {
		ads_netid = "";
		ads_port = 0;
		return false;
	}
	buf[255] = 0;
	std::stringcase	ads_netid_old = ads_netid;
	const int ads_port_old = ads_port;
	ads_netid = buf;
	ads_port = p;
	if (!isValid()) {
		ads_netid = ads_netid_old;
		ads_port = ads_port_old;
		return false;
	}
	return true;
}

/* compiler_info::set_cmpl_versionstr
 ************************************************************************/
void compiler_info::set_cmpl_versionstr (const std::stringcase& versionstr) 
{ 
	cmpl_versionstr = versionstr; 
	if (is_cmpl_Valid ()) {
		int v1, v2;
		sscanf_s (cmpl_versionstr.c_str(), "%i.%i.%*s", &v1, &v2);
		if (v2 > 99) {
			cmpl_version = v1 + (double)v2/1000;
		} 
		else if (v2 > 9) {
			cmpl_version = v1 + (double)v2/100;
		}
		else if (v2 > 0) {
			cmpl_version = v1 + (double)v2/10;
		}
		else {
			cmpl_version = 0;
		}
	}
	else {
		cmpl_version = 0;
	}
}

/* compiler_info::is_cmpl_Valid
 ************************************************************************/
bool compiler_info::is_cmpl_Valid() const noexcept
{
	if (cmpl_versionstr.empty()) {
		return false;
	}
	unsigned int v1 = 0;
	unsigned int v2 = 0;
	int end = 0;
	const int num = sscanf_s (cmpl_versionstr.c_str(), "%u.%u.%*s%n", 
						&v1, &v2, &end);
	return (num == 2);
}

/* compiler_info::set_tcat_versionstr
 ************************************************************************/
void compiler_info::set_tcat_versionstr (const std::stringcase& versionstr) 
{ 
	tcat_versionstr = versionstr; 
	if (is_tcat_Valid ()) {
		sscanf_s (tcat_versionstr.c_str(), "%u.%u.%u", &tcat_version_major, 
			&tcat_version_minor, &tcat_version_build);
	}
	else {
		tcat_version_major = 0;
		tcat_version_minor = 0;
		tcat_version_build = 0;
	}
}

/* compiler_info::is_tcat_Valid
 ************************************************************************/
bool compiler_info::is_tcat_Valid() const noexcept
{
	if (tcat_versionstr.empty()) {
		return false;
	}
	unsigned int v1 = 0;
	unsigned int v2 = 0;
	unsigned int v3 = 0;
	int end = 0;
	const int num = sscanf_s (tcat_versionstr.c_str(), "%u.%u.%u%n", 
						&v1, &v2, &v3, &end);
	return (num == 3);
}

/** compareNamesWoNamespace
 ************************************************************************/
bool compareNamesWoNamespace (const std::stringcase& p1, const std::stringcase& p2)
{
	const size_t pos1 = p1.length();
	const size_t pos2 = p2.length();
	if (pos1 > pos2) {
		return (p1[pos1 - pos2 - 1] == '.') &&
			(p1.compare (pos1 - pos2, std::stringcase::npos, p2) == 0);
	}
	else if (pos2 > pos1) {
		return (p2[pos2 - pos1 - 1] == '.') &&
			(p2.compare (pos2 - pos1, std::stringcase::npos, p1) == 0);
	}
	else {
		return p1 == p2;
	}
}

/* type_map::find
 ************************************************************************/
const type_map::value_type::second_type* 
type_map::find (value_type::first_type id, const std::stringcase& typn) const
{
	const_iterator t = end();
	const_iterator i = type_multipmap::find (id);
	while (i != end()) {
		if (i->first != id) {
			break;
		}
		if (compareNamesWoNamespace (i->second.get_name(), typn)) {
			t = i;
			break;
		}
		++i;
	}
	// fall back to linear search if id == 0, and not found with id == 0
	if ((t == end()) && (id == 0)) {
		i = begin();
		while (i != end()) {
			if (compareNamesWoNamespace(i->second.get_name(), typn)) {
				t = i;
				break;
			}
			++i;
		}
	}
	return (t == end()) ? nullptr : &t->second;
}

/* type_map::patch_type_decorators
 ************************************************************************/
int type_map::patch_type_decorators()
{
	int num = 0;
	unsigned int id = 0;
	// simple data type
	std::regex e(R"++(SINT|INT|DINT|LINT|USINT|UINT|UDINT|ULINT|BYTE|WORD|DWORD|LWORD|TIME|TOD|LTIME|DATE|DT|TIME_OF_DAY|DATE_AND_TIME|REAL|LREAL|BOOL|STRING|STRING\([0-9]+\))++");

	for (auto& i : *this)	{
		id = i.second.get_type_decoration();
		// check for zero id, array type and non-trivial type
		if (id != 0) continue;
		if (i.second.get_type_description() != type_enum::arraytype) continue;
		if (std::regex_match(i.second.get_type_name(), e)) continue;
		// find array subtype
		const type_record* t = find (id, i.second.get_type_name());
		if ((t != nullptr) && (t->get_name_decoration() != 0)) {
			i.second.set_type_decoration(t->get_name_decoration());
			++num;
		}
	}
	return num;
}

/* tpy_file::tpy_file
 ************************************************************************/
tpy_file::tpy_file (FILE* inp)
{
	parse (inp);
}

/* tpy_file::parse
 ************************************************************************/
bool tpy_file::parse (FILE* inp)
{
	if (!inp) {
		return false;
	}

	// Set up parser info
	parserinfo_type info (project_info, sym_list, type_list);

	// Initialize XML parser
	char buf[BUFSIZ] = "";
	XML_Parser parser = XML_ParserCreate (NULL);
	XML_SetUserData (parser, &info);
	XML_SetElementHandler (parser, startElement, endElement);
	XML_SetCdataSectionHandler (parser, startCData, endCData);
	XML_SetCharacterDataHandler (parser, dataElement);

	// read data and parse
	bool done = false;
	do {
		const int len = (int)fread(buf, 1, sizeof(buf), inp);
		done = len < sizeof(buf);
#pragma warning (disable : 26812)
		if (XML_Parse (parser, buf, len, done) == XML_Status::XML_STATUS_ERROR) {
			fprintf (stderr,
				"%s at line %" XML_FMT_INT_MOD "u\n",
				XML_ErrorString (XML_GetErrorCode (parser)),
				XML_GetCurrentLineNumber (parser));
			return false;
		}
#pragma warning (default : 26812)
	} while (!done);

	// Finish up
	XML_ParserFree(parser);
	parse_finish();
	return true;
}

/* tpy_file::parse
 ************************************************************************/
bool tpy_file::parse (const char* p, int len)
{
	if (!p || (len < 0)) {
		return false;
	}
	// Set up parser info
	parserinfo_type info (project_info, sym_list, type_list);

	// Initialize XML parser
	XML_Parser parser = XML_ParserCreate (NULL);
	XML_SetUserData (parser, &info);
	XML_SetElementHandler (parser, startElement, endElement);
	XML_SetCdataSectionHandler (parser, startCData, endCData);
	XML_SetCharacterDataHandler (parser, dataElement);

	// read data and parse
#pragma warning (disable : 26812)
	if (XML_Parse (parser, p, len, true) == XML_Status::XML_STATUS_ERROR) {
		fprintf (stderr,
			"%s at line %" XML_FMT_INT_MOD "u\n",
			XML_ErrorString (XML_GetErrorCode (parser)),
			XML_GetCurrentLineNumber (parser));
		return false;
	}
#pragma warning (default : 26812)

	// Finish up
	XML_ParserFree(parser);
	parse_finish();
	return true;
}

/* tpy_file::parse_finish
 ************************************************************************/
void tpy_file::parse_finish ()
{
	// patch missing type decorators
	const int num = type_list.patch_type_decorators();
	if (num > 0) {
		// fprintf(stderr, "Patching %d type decorators\n", num);
	}
	// set plc name in opc
	std::stringcase tcname = project_info.get();
	if (!tcname.empty()) {
		for (auto& sym : sym_list) {
			sym.get_opc().add (property_el (OPC_PROP_PLCNAME, tcname));
		}
	}
	//// patch export all
	//if (get_export_all()) {
	//	for (auto& sym : sym_list) {
	//		sym.get_opc().set_opc_state(ParseUtil::opc_enum::publish);
	//	}
	//}
}


/************************************************************************/
/* XML Parsing
 ************************************************************************/

/** XML get decoration number from attribute
 ************************************************************************/
bool get_decoration (const char **atts, unsigned int& decoration)
{
	for (const char** pp = atts; pp && pp[0] && pp[1]; pp += 2) {
		std::stringcase a (pp[0]);
		if (a.compare (xmlAttrDecoration) == 0) {
			decoration = strtol (pp[1], NULL, 16);
			return true;
		}
	}
	return false;
}

/** XML get pointer from attribute
 ************************************************************************/
bool get_pointer (const char **atts)
{
	for (const char** pp = atts; pp && pp[0] && pp[1]; pp += 2) {
		std::stringcase a (pp[0]);
		if (a.compare (xmlAttrPointer) == 0) {
			std::stringcase val (pp[1]);
			return (val == "true") || (val == "t") || (val == "1");
		}
	}
	return false;
}
   
/* XML start element function callback
 ************************************************************************/
static void XMLCALL startElement (void *userData, const char *name, 
	const char **atts)
{
	parserinfo_type*	pinfo = (parserinfo_type*) userData;
	if (pinfo->ignore) {
		++pinfo->ignore;
		return;
	}
	std::stringcase n (name ? name : "");

	// Parse PLC project information
	if (pinfo->verytop() && (n.compare (xmlPlcProjectInfo) == 0)) {
		pinfo->projects = true;
	}

	// Parse routing information
	else if (n.compare (xmlRoutingInfo) == 0) {
		if (pinfo->top()) {
			++pinfo->routing;
		}
		else ++pinfo->ignore;
	}
	else if (n.compare (xmlAdsInfo) == 0) {
		if (pinfo->routing == 1) {
			++pinfo->routing;
		}
		else ++pinfo->ignore;
	}
	else if (pinfo->routing >= 2) {
		// Net Id
		if (n.compare (xmlNetId) == 0 && (pinfo->routing == 2)) {
			pinfo->data = std::stringcase ("");
			pinfo->routing = 3;
		}
		// Port
		else if (n.compare (xmlPort) == 0 && (pinfo->routing == 2)) {
			pinfo->data = std::stringcase ("");
			pinfo->routing = 4;
		}
		// Target name
		else if (n.compare (xmlTargetName) == 0 && (pinfo->routing == 2)) {
			pinfo->data = std::stringcase ("");
			pinfo->routing = 5;
		}
	}

	// Parse compiler information
	else if (n.compare (xmlCompilerInfo) == 0) {
		if (pinfo->top()) {
			++pinfo->compiler;
		}
		else ++pinfo->ignore;
	}
	else if (pinfo->compiler >= 1) {
		// Net Id
		if (n.compare (xmlCompilerVersion) == 0 && (pinfo->compiler == 1)) {
			pinfo->data = std::stringcase ("");
			pinfo->compiler = 2;
		}
		// Port
		else if (n.compare (xmlTwinCATVersion) == 0 && (pinfo->compiler == 1)) {
			pinfo->data = std::stringcase ("");
			pinfo->compiler = 3;
		}
		// CPU Family
		else if (n.compare (xmlCpuFamily) == 0 && (pinfo->compiler == 1)) {
			pinfo->data = std::stringcase ("");
			pinfo->compiler = 4;
		}
	}

	// Parse symbol information
	else if (n.compare (xmlSymbols) == 0) {
		if (pinfo->top()) {
			++pinfo->symbols;
		}
		else ++pinfo->ignore;
	}
	else if (n.compare (xmlSymbol) == 0) {
		if (pinfo->symbols == 1) {
			++pinfo->symbols;
			pinfo->init();
		}
		else ++pinfo->ignore;
	}
	else if (pinfo->symbols == 2) {
		// name of symbol
		if (n.compare (xmlName) == 0 && !pinfo->name_parse && 
			!pinfo->opc_parse) {
				pinfo->name_parse = 2;
		}
		// type of symbol
		else if (n.compare (xmlType) == 0 && !pinfo->type_parse) {
			pinfo->type_parse = 2;
			unsigned int decor = 0;
			get_decoration (atts, decor);
			pinfo->sym.set_type_decoration (decor);
			pinfo->sym.set_type_pointer (get_pointer (atts));
		}
		// opc properties
		else if (n.compare (xmlProperties) == 0 && !pinfo->opc_parse &&
			pinfo->name_parse <= 1 && pinfo->type_parse <= 1) {
				pinfo->opc_parse = 1;
		}
		// opc property
		else if (n.compare (xmlProperty) == 0 && pinfo->opc_parse == 1) {
			pinfo->opc_parse = 2;
			pinfo->opc_prop = property_el ();
		}
		// opc property name
		else if (n.compare (xmlName) == 0 && pinfo->opc_parse == 2) {
			pinfo->opc_parse = 3;
			pinfo->opc_data = "";
			pinfo->opc_cdata = 1;
		}
		// opc property value
		else if (n.compare (xmlValue) == 0 && pinfo->opc_parse == 2) {
			pinfo->opc_parse = 4;
			pinfo->opc_data = "";
			pinfo->opc_cdata = 1;
		}
		// igroup
		else if (n.compare (xmlIGroup) == 0 && !pinfo->igroup_parse) {
			pinfo->igroup_parse = 2;
			pinfo->data = std::stringcase ("");
		}
		// ioffset
		else if (n.compare (xmlIOffset) == 0 && !pinfo->ioffset_parse) {
			pinfo->ioffset_parse = 2;
			pinfo->data = std::stringcase ("");
		}
		// bitsize
		else if (n.compare (xmlBitSize) == 0 && !pinfo->bitsize_parse) {
			pinfo->bitsize_parse = 2;
			pinfo->data = std::stringcase ("");
		}
		else {
			++pinfo->ignore;
		}
	}

	// Parse data type information
	else if (n.compare (xmlDataTypes) == 0) {
		if (pinfo->top()) {
			++pinfo->types;
		}
		else ++pinfo->ignore;
	}
	else if (n.compare (xmlDataType) == 0) {
		if (pinfo->types == 1) {
			++pinfo->types;
			pinfo->init();
		}
		else ++pinfo->ignore;
	}
	else if (pinfo->types == 2) {
		// name of type
		if (n.compare (xmlName) == 0 && !pinfo->name_parse && 
			(pinfo->struct_parse <= 1) && !pinfo->opc_cur) {
				pinfo->name_parse = 2;
				unsigned int decor = 0;
				get_decoration (atts, decor);
				pinfo->rec.set_name_decoration (decor);
		}
		// right hand type
		else if (n.compare (xmlType) == 0 && !pinfo->type_parse && 
			pinfo->struct_parse <= 1) {
				pinfo->type_parse = 2;
				unsigned int decor = 0;
				get_decoration (atts, decor);
				pinfo->rec.set_type_decoration (decor);
		}
		// bit size
		else if (n.compare (xmlBitSize) == 0 && !pinfo->bitsize_parse &&
			pinfo->struct_parse <= 1) {
				pinfo->bitsize_parse = 2;
				pinfo->data = std::stringcase ("");
		}
		// array info
		else if (n.compare (xmlArrayInfo) == 0) {
			pinfo->array_parse = 2;
			pinfo->array_bounds = dimension (0, 0);
		}
		// lower array bound
		else if (n.compare (xmlArrayLBound) == 0 && 
			pinfo->array_parse == 2) {
				pinfo->array_parse = 3;
				pinfo->array_data = std::stringcase ("");
		}
		// array elements
		else if (n.compare (xmlArrayElements) == 0 && 
			pinfo->array_parse == 2) {
				pinfo->array_parse = 3;
				pinfo->array_data = std::stringcase ("");
		}
		// enum
		else if (n.compare (xmlEnumInfo) == 0) {
			pinfo->enum_parse = 2;
			pinfo->enum_element = enum_pair (0, "");
			pinfo->enum_comment.clear();
		}
		// enum tag
		else if (n.compare (xmlEnumEnum) == 0 && 
			pinfo->enum_parse == 2) {
				pinfo->enum_parse = 3;
				pinfo->enum_data = std::stringcase ("");
		}
		// enum text
		else if (n.compare (xmlEnumText) == 0 && 
			pinfo->enum_parse == 2) {
				pinfo->enum_parse = 3;
				pinfo->enum_data = std::stringcase ("");
		}
		// enum comment
		else if (n.compare (xmlEnumComment) == 0 && 
			pinfo->enum_parse == 2) {
				pinfo->enum_parse = 3;
				pinfo->enum_data = std::stringcase ("");
		}
		// structure
		else if (n.compare (xmlSubItem) == 0) {
			pinfo->struct_parse = 2;
			pinfo->struct_element = item_record();
		}
		// structure element name
		else if (n.compare (xmlName) == 0 && pinfo->struct_parse == 2 && 
			!pinfo->opc_cur) {
				pinfo->struct_parse = 3;
		}
		// structure element type
		else if (n.compare (xmlType) == 0 && pinfo->struct_parse == 2 && 
			!pinfo->opc_cur) {
				pinfo->struct_parse = 4;
				unsigned int decor = 0;
				get_decoration (atts, decor);
				pinfo->struct_element.set_type_decoration (decor);
		}
		// structure element bitsize
		else if (n.compare (xmlBitSize) == 0 && pinfo->struct_parse == 2 && 
			!pinfo->opc_cur) {
				pinfo->struct_parse = 5;
				pinfo->data = std::stringcase ("");
		}
		// structure element bitoffs
		else if (n.compare (xmlBitOffs) == 0 && pinfo->struct_parse == 2 && 
			!pinfo->opc_cur) {
				pinfo->struct_parse = 5;
				pinfo->data = std::stringcase ("");
		}
		// function block
		else if (n.compare (xmlFbInfo) == 0) {
			pinfo->fb_parse = 1;
			++pinfo->ignore;
		}
		// opc properties
		else if (n.compare (xmlProperties) == 0 && !pinfo->opc_cur &&
			pinfo->name_parse <= 1 &&
			pinfo->type_parse <= 1 &&
			pinfo->enum_parse <= 1 &&
			pinfo->array_parse <= 1 &&
			pinfo->struct_parse <= 2) {
				pinfo->opc_parse = 1;
				if (pinfo->struct_parse == 2) 
					pinfo->opc_cur = &pinfo->struct_element.get_opc();
				else pinfo->opc_cur = &pinfo->rec.get_opc();
		}
		// opc property
		else if (n.compare (xmlProperty) == 0 && pinfo->opc_cur &&
			pinfo->opc_parse == 1) {
				pinfo->opc_parse = 2;
				pinfo->opc_prop = property_el ();
		}
		// opc property name
		else if (n.compare (xmlName) == 0 && pinfo->opc_cur &&
			pinfo->opc_parse == 2) {
				pinfo->opc_parse = 3;
				pinfo->opc_data = "";
				pinfo->opc_cdata = 1;
		}
		// opc property value
		else if (n.compare (xmlValue) == 0 && pinfo->opc_cur &&
			pinfo->opc_parse == 2) {
				pinfo->opc_parse = 4;
				pinfo->opc_data = "";
				pinfo->opc_cdata = 1;
		}
		else {
			++pinfo->ignore;
		}
	}
	else {
		++pinfo->ignore;
	}
}

/* XML end element function callback
 ************************************************************************/
static void XMLCALL endElement (void *userData, const char *name)
{
	parserinfo_type*	pinfo = (parserinfo_type*) userData;
	if (pinfo->ignore) {
		--pinfo->ignore;
		return;
	}
	std::stringcase n (name ? name : "");

	// parsing PLC project information
	if (n.compare (xmlPlcProjectInfo) == 0) {
		if (pinfo->top()) pinfo->projects = false;
	}

	// Parse routing information
	else if (n.compare (xmlRoutingInfo) == 0) {
		if (pinfo->routing == 1) --pinfo->routing;
	}
	else if (n.compare (xmlAdsInfo) == 0) {
		if (pinfo->routing == 2) --pinfo->routing;
	}
	else if (pinfo->routing >= 2) {
		// Net Id
		if (n.compare (xmlNetId) == 0 && (pinfo->routing == 3)) {
			pinfo->routing = 2;
			trim_space (pinfo->data);
			pinfo->get_projectinfo().set_netid(pinfo->data);
		}
		// Port
		else if (n.compare (xmlPort) == 0 && (pinfo->routing == 4)) {
			pinfo->routing = 2;
			const int num = strtol (pinfo->data.c_str(), NULL, 10);
			pinfo->get_projectinfo().set_port (num);
		}
		// Target name
		else if (n.compare (xmlTargetName) == 0 && (pinfo->routing == 5)) {
			pinfo->routing = 2;
			trim_space (pinfo->data);
			pinfo->get_projectinfo().set_targetname(pinfo->data);
		}
	}

	// Parse compiler information
	else if (n.compare (xmlCompilerInfo) == 0) {
		if (pinfo->compiler == 1) --pinfo->compiler;
	}
	else if (pinfo->compiler >= 1) {
		// compiler version
		if (n.compare (xmlCompilerVersion) == 0 && (pinfo->compiler == 2)) {
			pinfo->compiler = 1;
			trim_space (pinfo->data);
			pinfo->get_projectinfo().set_cmpl_versionstr(pinfo->data);
		}
		// twincat version
		else if (n.compare (xmlTwinCATVersion) == 0 && (pinfo->compiler == 3)) {
			pinfo->compiler = 1;
			trim_space (pinfo->data);
			pinfo->get_projectinfo().set_tcat_versionstr (pinfo->data);
		}
		// CPU family
		else if (n.compare (xmlCpuFamily) == 0 && (pinfo->compiler == 4)) {
			pinfo->compiler = 1;
			trim_space (pinfo->data);
			pinfo->get_projectinfo().set_cpu_family (pinfo->data);
		}
	}

	// parsing symbols
	else if (n.compare (xmlSymbols) == 0) {
		if (pinfo->symbols == 1) --pinfo->symbols;
	}
	else if (n.compare (xmlSymbol) == 0) {
		if (pinfo->symbols == 2) {
			--pinfo->symbols;
			if (!pinfo->sym.get_name().empty()) {
				// pointers are readonly!
				if (pinfo->sym.get_type_pointer()) {
					pinfo->sym.get_opc().get_properties()[OPC_PROP_RIGHTS] = "1";
				}
				pinfo->get_symbols().push_back (pinfo->sym);
			}
		}
	}
	else if (pinfo->symbols == 2) {
		// parsed a name (trim space)
		if (n.compare (xmlName) == 0 && pinfo->name_parse == 2) {
			pinfo->name_parse = 1;
			trim_space (pinfo->sym.get_name());
		}
		// parsed a type (trim space)
		else if (n.compare (xmlType) == 0 && pinfo->type_parse == 2) {
			pinfo->type_parse = 1;
			trim_space (pinfo->sym.get_type_name());
		}
		// opc properties
		else if (n.compare (xmlProperties) == 0 && pinfo->opc_parse == 1) {
			pinfo->opc_parse = 0;
		}
		// opc property
		else if (n.compare (xmlProperty) == 0 && pinfo->opc_parse == 2) {
			pinfo->opc_parse = 1;
			if (pinfo->opc_prop.first == -1) {
				const int num = strtol (pinfo->opc_prop.second.c_str(), NULL, 10);
				pinfo->sym.get_opc().set_opc_state (num ? opc_enum::publish : opc_enum::silent);
			}
			else if (pinfo->opc_prop.first > 0) {
				pinfo->sym.get_opc().add (pinfo->opc_prop);
			}
		}
		// opc property name
		else if (n.compare (xmlName) == 0 && pinfo->opc_parse == 3) {
			pinfo->opc_parse = 2;
			trim_space (pinfo->opc_data);
			int num = 0;
			if (pinfo->opc_data.compare (opcExport) == 0) num = -1;
			else if (pinfo->opc_data.compare (0, 8, opcProp) == 0) {
				pinfo->opc_data.erase (0, 8);
				trim_space (pinfo->opc_data);
				if (pinfo->opc_data.compare (0, 1, opcBracket) == 0) 
					pinfo->opc_data.erase (0, 1);
				num = strtol (pinfo->opc_data.c_str(), NULL, 10);
			}
			pinfo->opc_prop.first = num;
		}
		// opc property value
		else if (n.compare (xmlValue) == 0 && pinfo->opc_parse == 4) {
			pinfo->opc_parse = 2;
			pinfo->opc_prop.second = pinfo->opc_data;
		}
		// parsed a igroup (convert to integer)
		else if (n.compare (xmlIGroup) == 0 && pinfo->igroup_parse == 2) {
			pinfo->igroup_parse = 1;
			pinfo->sym.set_igroup (strtol (pinfo->data.c_str(), NULL, 10));
		}
		// parsed a ioffset (convert to integer)
		else if (n.compare (xmlIOffset) == 0 && pinfo->ioffset_parse == 2) {
			pinfo->ioffset_parse = 1;
			pinfo->sym.set_ioffset (strtol (pinfo->data.c_str(), NULL, 10));
		}
		// parsed a bitsize (convert to integer)
		else if (n.compare (xmlBitSize) == 0 && pinfo->bitsize_parse == 2) {
			pinfo->bitsize_parse = 1;
			pinfo->sym.set_bytesize (strtol (pinfo->data.c_str(), NULL, 10) / 8);
		}
	}
	// parsing data types
	else if (n.compare (xmlDataTypes) == 0) {
		if (pinfo->types == 1) --pinfo->types;
	}
	else if (n.compare (xmlDataType) == 0) {
		if (pinfo->types == 2) {
			--pinfo->types;
			pinfo->rec.set_type_description (pinfo->get_type_description());
			// remove simple type which reference themselves, ie., discard type aliases
			if ((pinfo->rec.get_type_description() != type_enum::simple) ||
				(pinfo->rec.get_name() != pinfo->rec.get_type_name())) {
				pinfo->get_types().insert (
				type_map::value_type (pinfo->rec.get_name_decoration(), pinfo->rec));
			}
		}
	}
	else if (pinfo->types == 2) {
		// parsed a name (trim space)
		if (n.compare (xmlName) == 0 && pinfo->name_parse == 2) {
			pinfo->name_parse = 1;
			trim_space (pinfo->rec.get_name());
		}
		// parsed a type (trim space)
		else if (n.compare (xmlType) == 0 && pinfo->type_parse == 2) {
			pinfo->type_parse = 1;
			trim_space (pinfo->rec.get_type_name());
		}
		// parsed a bit size
		else if (n.compare (xmlBitSize) == 0 && pinfo->bitsize_parse == 2) {
			pinfo->bitsize_parse = 1;
			pinfo->rec.set_bit_size (strtol (pinfo->data.c_str(), NULL, 10));
		}
		// parsed an array info
		else if (n.compare (xmlArrayInfo) == 0 && pinfo->array_parse == 2) {
			pinfo->array_parse = 1;
			if (pinfo->array_bounds.second > 0) {
				pinfo->rec.get_array_dimensions().push_back (pinfo->array_bounds);
			}
		}
		// lower array bound
		else if (n.compare (xmlArrayLBound) == 0 && 
			pinfo->array_parse == 3) {
				pinfo->array_bounds.first = 
					strtol (pinfo->array_data.c_str(), NULL, 10);
				pinfo->array_parse = 2;
		}
		// array elements
		else if (n.compare (xmlArrayElements) == 0 && 
			pinfo->array_parse == 3) {
				pinfo->array_bounds.second = 
					strtol (pinfo->array_data.c_str(), NULL, 10);
				pinfo->array_parse = 2;
		}
		// parsed an enum info
		else if (n.compare (xmlEnumInfo) == 0 && pinfo->enum_parse == 2) {
			pinfo->enum_parse = 1;
			pinfo->rec.get_enum_list().insert (pinfo->enum_element);
		}
		// enum tag
		else if (n.compare (xmlEnumEnum) == 0 && 
			pinfo->enum_parse == 3) {
				pinfo->enum_element.first = 
					strtol (pinfo->enum_data.c_str(), NULL, 10);
				pinfo->enum_parse = 2;
		}
		// enum text
		else if (n.compare (xmlEnumText) == 0 && 
			pinfo->enum_parse == 3) {
				trim_space (pinfo->enum_data);
				pinfo->enum_element.second = pinfo->enum_data;
				pinfo->enum_parse = 2;
		}
		// enum comment
		else if (n.compare (xmlEnumComment) == 0 && 
			pinfo->enum_parse == 3) {
				trim_space (pinfo->enum_data);
				pinfo->enum_comment = pinfo->enum_data;
				pinfo->enum_parse = 2;
		}
		// parsed a subitem
		else if (n.compare (xmlSubItem) == 0 && pinfo->struct_parse == 2) {
			pinfo->struct_parse = 1;
			pinfo->rec.get_struct_list().push_back (pinfo->struct_element);
		}
		// subitem name
		else if (n.compare (xmlName) == 0 && pinfo->struct_parse == 3) {
			trim_space (pinfo->struct_element.get_name());
			pinfo->struct_parse = 2;
		}
		// subitem type
		else if (n.compare (xmlType) == 0 && pinfo->struct_parse == 4) {
			trim_space (pinfo->struct_element.get_type_name());
			pinfo->struct_parse = 2;
		}
		// subitem bitsize
		else if (n.compare (xmlBitSize) == 0 && pinfo->struct_parse == 5) {
			pinfo->struct_element.set_bit_size (
				strtol (pinfo->data.c_str(), NULL, 10));
			pinfo->struct_parse = 2;
		}
		// subitem bitoffs
		else if (n.compare (xmlBitOffs) == 0 && pinfo->struct_parse == 5) {
			pinfo->struct_element.set_bit_offset (
				strtol (pinfo->data.c_str(), NULL, 10));
			pinfo->struct_parse = 2;
		}
		// opc properties
		else if (n.compare (xmlProperties) == 0 && pinfo->opc_parse == 1) {
			pinfo->opc_parse = 0;
			pinfo->opc_cur = 0;
		}
		// opc property
		else if (n.compare (xmlProperty) == 0 && pinfo->opc_parse == 2) {
			pinfo->opc_parse = 1;
			if (pinfo->opc_prop.first == -1) {
				const int num = strtol (pinfo->opc_prop.second.c_str(), NULL, 10);
				if (pinfo->opc_cur) 
					pinfo->opc_cur->set_opc_state (num ? opc_enum::publish : opc_enum::silent);
			}
			else if (pinfo->opc_prop.first > 0) {
				if (pinfo->opc_cur) 
					pinfo->opc_cur->get_properties().insert (pinfo->opc_prop);
			}
		}
		// opc property name
		else if (n.compare (xmlName) == 0 && pinfo->opc_parse == 3) {
			pinfo->opc_parse = 2;
			trim_space (pinfo->opc_data);
			int num = 0;
			if (pinfo->opc_data.compare (opcExport) == 0) num = -1;
			else if (pinfo->opc_data.compare (0, 8, opcProp) == 0) {
				pinfo->opc_data.erase (0, 8);
				trim_space (pinfo->opc_data);
				if (pinfo->opc_data.compare (0, 1, opcBracket) == 0) 
					pinfo->opc_data.erase (0, 1);
				num = strtol (pinfo->opc_data.c_str(), NULL, 10);
			}
			pinfo->opc_prop.first = num;
		}
		// opc property value
		else if (n.compare (xmlValue) == 0 && pinfo->opc_parse == 4) {
			pinfo->opc_parse = 2;
			pinfo->opc_prop.second = pinfo->opc_data;
		}
	}
}

/* XML start CData function callback
 ************************************************************************/
static void XMLCALL startCData (void *userData) noexcept
{
	parserinfo_type*	pinfo = (parserinfo_type*) userData;
	if (pinfo->ignore) {
		return;
	}
	if (pinfo->symbols == 2) {
		// clear opc data
		if (pinfo->opc_parse >= 3) {
			pinfo->opc_data.clear ();
			pinfo->opc_cdata = 1;
		}
	}
	else if (pinfo->types == 2) {
		// clear opc data
		if (pinfo->opc_parse >= 3) {
			pinfo->opc_data.clear ();
			pinfo->opc_cdata = 1;
		}
	}
}

/* XML end CData function callback
 ************************************************************************/
static void XMLCALL endCData (void *userData) noexcept
{
	parserinfo_type*	pinfo = (parserinfo_type*) userData;
	if (pinfo->ignore) {
		return;
	}
	if (pinfo->symbols == 2) {
		// clear opc data
		if (pinfo->opc_parse >= 3) {
			pinfo->opc_cdata = 0;
		}
	}
	else if (pinfo->types == 2) {
		// clear opc data
		if (pinfo->opc_parse >= 3) {
			pinfo->opc_cdata = 0;
		}
	}
}


/* XML data function callback
 ************************************************************************/
static void XMLCALL dataElement (void *userData, const char *data, int len)
{
	parserinfo_type*	pinfo = (parserinfo_type*) userData;
	if (pinfo->ignore) {
		return;
	}

	// get data from routing
	if (pinfo->routing >= 2) {
		pinfo->data.append (data, len);
	}
	// get data from compiler
	else if (pinfo->compiler >= 1) {
		pinfo->data.append (data, len);
	}

	// get symbol data
	else if (pinfo->symbols == 2) {
		// append string to name
		if (pinfo->name_parse == 2) {
			pinfo->sym.get_name().append (data, len);
		}
		// append string to type
		else if (pinfo->type_parse == 2) {
			pinfo->sym.get_type_name().append (data, len);
		}
		// append opc data
		else if (pinfo->opc_parse >= 3) {
			if (pinfo->opc_cdata) 
				pinfo->opc_data.append (data, len);
		}
		// append igroup/ioffset/bitsize data
		else if ((pinfo->igroup_parse == 2) ||
			(pinfo->ioffset_parse == 2) ||
			(pinfo->bitsize_parse == 2)) {
				pinfo->data.append (data, len);
		}
	}

	// get data type information
	else if (pinfo->types == 2) {
		// append string to name
		if (pinfo->name_parse == 2) {
			pinfo->rec.get_name().append (data, len);
		}
		// append string to type
		else if (pinfo->type_parse == 2) {
			pinfo->rec.get_type_name().append (data, len);
		}
		// append string to bitdata
		else if (pinfo->bitsize_parse == 2) {
			pinfo->data.append (data, len);
		}
		// append array info data
		else if (pinfo->array_parse == 3) {
			pinfo->array_data.append (data, len);
		}
		// append enum data
		else if (pinfo->enum_parse == 3) {
			pinfo->enum_data.append (data, len);
		}
		// append struct element name
		else if (pinfo->struct_parse == 3) {
			pinfo->struct_element.get_name().append (data, len);
		}
		// append struct element type
		else if (pinfo->struct_parse == 4) {
			pinfo->struct_element.get_type_name().append (data, len);
		}
		// append struct element bitsize
		else if (pinfo->struct_parse == 5) {
			pinfo->data.append (data, len);
		}
		// append struct element bitoffs
		else if (pinfo->struct_parse == 6) {
			pinfo->data.append (data, len);
		}
		// append opc data
		else if (pinfo->opc_parse >= 3) {
			if (pinfo->opc_cdata) 
				pinfo->opc_data.append (data, len);
		}
	}
}

}