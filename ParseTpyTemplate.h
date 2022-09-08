/** @file ParseTpyTemplate.h
	Templates for functions for parsing tpy file.
 ************************************************************************/
namespace ParseTpy {

/* tpy_file::process_symbols
 ************************************************************************/
	template <class Function>
	int tpy_file::process_symbols (Function& process, 
		const std::stringcase& prefix) const
	{
		int num = 0;
		for (const auto& sym : get_symbols()) {
			if (get_export_all() || 
				(sym.get_opc().get_opc_state() == ParseUtil::opc_enum::publish)) {
				num += process_type_tree (sym, process, prefix);
			}
		}
		return num;
	}
	
/* tpy_file::process_type_tree
 ************************************************************************/
	template <class Function>
	int tpy_file::process_type_tree (const symbol_record& symbol, 
		Function& process, const std::stringcase& prefix) const
	{
		ParseUtil::variable_name n (prefix);
		n.append (symbol.get_name(), symbol.get_opc(), "");
		// Check for pointer: if so, do not follow
		if (symbol.get_type_pointer()) {
			// check for 64 bit pointers
			if (symbol.get_bytesize() == 8) {
				return process_type_tree("ULINT", 0,
					symbol.get_opc(), symbol, process, n, 0);
			}
			else {
				return process_type_tree("UDINT", 0,
					symbol.get_opc(), symbol, process, n, 0);
			}
		}
		else if (!n.get_name().empty()) {
			ParseUtil::opc_list opc(symbol.get_opc());
			return process_type_tree (symbol.get_type_name(), 
				symbol.get_type_decoration(), 
				opc, symbol, process, n, 0);
		}
		else {
			return 0;
		}
	}

/* tpy_file::process_type_tree
 ************************************************************************/
	template <class Function>
	int tpy_file::process_type_tree (const type_record& typ, 
		ParseUtil::opc_list defopc, const ParseUtil::memory_location& loc, 
		Function& process, const ParseUtil::variable_name& varname, 
		int level) const
	{
		// Check recursive level
		if (level > 100) return 0;

		// need to make sure that we don't add opc definitions when the array type was
		// implicitly defined through a declaration of form A : ARRAY[n...m] OF ...
		// multiple implicit array types of the same index range and base type will
		// share a single type defintion and therefore share the opc definitions of 
		// the first implicit one
		if ((typ.get_type_description() != type_enum::arraytype) ||
			(typ.get_name().find ('[') == std::stringcase::npos)) {
			defopc.add (typ.get_opc());
		}
		switch (typ.get_type_description()) {
		case type_enum::simple :
			{
				return process_type_tree (typ.get_type_name(), 
					typ.get_type_decoration(), defopc, loc, process, varname, level);
			}
		case type_enum::arraytype :
			// process array and iterate over all subindices
			return process_array (typ, typ.get_array_dimensions(), defopc, loc, 
				process, varname, level);
		case type_enum::enumtype :
			if (get_process_tags() == ParseUtil::process_tag_enum::atomic ||
				get_process_tags() == ParseUtil::process_tag_enum::all) {
				// check if enum is contained with 0 to 15
				bool withinhex = true;
				int min = 0;
				int max = -1;
				for (const auto& e : typ.get_enum_list()) {
						if ((e.first < 0) || (e.first >= 16)) {
							withinhex = false;
						}
						if (min > max) {
							min = max = e.first;
						}
						else if (e.first < min) {
							min = e.first;
						}
						else if (e.first > max) {
							max = e.first;
						}
				}
				// if not, treat as int
				if (!withinhex) {
					// add HOPR/LOPR
					if (max >= min) {
						defopc.get_properties().insert (ParseUtil::property_el (102, std::to_string (max).c_str()));
						defopc.get_properties().insert (ParseUtil::property_el (103, std::to_string (min).c_str()));
					}
					ParseUtil::process_arg_tc arg (loc, varname, ParseUtil::process_type_enum::pt_int, defopc, typ.get_type_name(), true);
					return process (arg) ? 1 : 0;
				}
				// add opc property for enum values
				else {
 					// add opc property for enum values
					for (const auto& e : typ.get_enum_list()) {
							defopc.get_properties().insert (
								ParseUtil::property_el (8510 + e.first, e.second));
					}
					ParseUtil::process_arg_tc arg (loc, varname, ParseUtil::process_type_enum::pt_enum, defopc, typ.get_type_name(), true);
					return process (arg) ? 1 : 0;
				}
			}
			else {
				return 0;
			}
		case type_enum::structtype :
		case type_enum::functionblock :
			{
				// Call process for entire struct (not an atomic type)
				int num = 0;
				if (get_process_tags() == ParseUtil::process_tag_enum::structured ||
					get_process_tags() == ParseUtil::process_tag_enum::all) {
					ParseUtil::process_arg_tc arg (loc, varname, ParseUtil::process_type_enum::pt_binary, defopc, typ.get_name(), false);
					num = process (arg) ? 1 : 0;
				}
				// iterate over all structure items
				for (const auto& i : typ.get_struct_list()) {
					// calculate sub element location
					ParseUtil::memory_location el_loc = loc;
					if (!el_loc.set_section (i)) {
						continue;
					}
					// Add a dot to the variable name
					ParseUtil::variable_name n (varname);
					n.append (i.get_name(), i.get_opc(), "."); 
					ParseUtil::opc_list o (defopc);
					o.add (i.get_opc());
					num += process_type_tree (i.get_type_name(), 
						i.get_type_decoration(), o, el_loc, process, n, level + 1);
				}
				return num; 
			}
		default :
			fprintf (stderr, "Unknown type %s for %s\n", typ.get_name().c_str(), varname.get_name().c_str());
			return 0;
		}
	}

/* tpy_file::process_type_tree
 ************************************************************************/
	template <class Function>
	int tpy_file::process_type_tree (const std::stringcase& typ, unsigned int id,
		const ParseUtil::opc_list& defopc, const ParseUtil::memory_location& loc, 
		Function& process, const ParseUtil::variable_name& varname, int level) const
	{
		const type_record* t = nullptr;

		if ((typ == "SINT")  || (typ == "INT")  || (typ == "DINT")  || (typ == "LINT")  ||
			(typ == "USINT") || (typ == "UINT") || (typ == "UDINT") || (typ == "ULINT") ||
			(typ == "BYTE")  || (typ == "WORD") || (typ == "DWORD") || (typ == "LWORD") ||
			(typ == "TIME")  || (typ == "TOD")  || (typ == "LTIME") || (typ == "DATE")  ||
			(typ == "DT")    || (typ == "TIME_OF_DAY") || (typ == "DATE_AND_TIME")) {
			if (get_process_tags() == ParseUtil::process_tag_enum::atomic ||
				get_process_tags() == ParseUtil::process_tag_enum::all) {
				ParseUtil::process_arg_tc arg (loc, varname, ParseUtil::process_type_enum::pt_int, defopc, typ, true);
				return process (arg) ? 1 : 0;
			}
			else return 0;
		}
		else if (typ == "REAL" || typ == "LREAL") {
			if (get_process_tags() == ParseUtil::process_tag_enum::atomic ||
				get_process_tags() == ParseUtil::process_tag_enum::all) {
				ParseUtil::process_arg_tc arg (loc, varname, ParseUtil::process_type_enum::pt_real, defopc, typ, true);
				return process (arg) ? 1 : 0;
			}
			else return 0;
		}
		else if (typ == "BOOL") {
			if (get_process_tags() == ParseUtil::process_tag_enum::atomic ||
				get_process_tags() == ParseUtil::process_tag_enum::all) {
				ParseUtil::process_arg_tc arg (loc, varname, ParseUtil::process_type_enum::pt_bool, defopc, typ, true);
				return process (arg) ? 1 : 0;
			}
			else return 0;
		}
		else if (typ.compare (0, 6, "STRING") == 0) {
			if ((get_process_tags() == ParseUtil::process_tag_enum::atomic ||
				 get_process_tags() == ParseUtil::process_tag_enum::all) &&
				 !get_no_strings()) {
				ParseUtil::process_arg_tc arg (loc, varname, ParseUtil::process_type_enum::pt_string, defopc, typ, true);
				return process (arg) ? 1 : 0;
			}
			else return 0;
		}
		else if ((t = type_list.find (id, typ)) != nullptr) {
			return process_type_tree (*t, defopc, loc, process, varname, level);
		}
		else {
			fprintf (stderr, "Unknown type %s for %s\n", typ.c_str(), varname.get_name().c_str());
			return 0;
		}
	}

/* tpy_file::process_array
 ************************************************************************/
	template <class Function>
	int tpy_file::process_array (const type_record& typ, dimensions dim, 
		const ParseUtil::opc_list& defopc, const ParseUtil::memory_location& loc, 
		Function& process, const ParseUtil::variable_name& varname, int level) const
	{
		// This is an array where all dimensions have been processed
		if (dim.empty()) {
			return process_type_tree (typ.get_type_name(), 
				typ.get_type_decoration(), defopc, loc, process, 
				varname, level);
		}
		// If not process through the indices
		else {
			const dimension d = dim.front();
			dim.pop_front();
			// invalid number of elements
			if (d.second < 0) {
				fprintf (stderr, "Array with negative element number for %s\n", 
						 varname.get_name().c_str());
				return 0;
			}
			// Call process for entire array (not an atomic type)
			ParseUtil::process_arg_tc arg (loc, varname, ParseUtil::process_type_enum::pt_binary, defopc, typ.get_name(), false);
			int num = 0;
			if (get_process_tags() == ParseUtil::process_tag_enum::structured ||
				get_process_tags() == ParseUtil::process_tag_enum::all) {
				num += process (arg) ? 1 : 0;
			}

			// no elements
			if (d.second == 0) {
				// just ignore and continue
				return 0;
			}
			// valid number of elements
			else {
				// Check if we have a size which is a multiple of the array length
				if (typ.get_bit_size() % d.second != 0) {
					fprintf (stderr, "Illegal array bit size for %s\n", 
						     varname.get_name().c_str());
					return 0;
				}
			}
			// loop through first array dimension and call process_array for next dimension
			const int el_bitsize = typ.get_bit_size() / d.second;
			// create a type with bit size = row size
			type_record ntyp (typ);
			ntyp.set_bit_size (el_bitsize);
			for (int i = d.first; i < d.first + d.second; ++i) {
				ParseUtil::memory_location el_loc = loc;
				const ParseUtil::bit_location el ((i - d.first) * el_bitsize, el_bitsize);
				if (!el_loc.set_section (el)) {
					continue;
				}
				char buf[40];
				sprintf_s (buf, sizeof (buf), "[%i]", i);
				ParseUtil::variable_name narr (varname);
				narr.append (buf, "");
				num += process_array (ntyp, dim, defopc, el_loc, process, narr, level);
			}
			return num;
		}
	}

}
