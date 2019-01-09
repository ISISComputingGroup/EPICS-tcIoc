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
		for (symbol_list::const_iterator sym = get_symbols().begin();
			sym != get_symbols().end(); ++sym, ++num) {
			if (get_export_all() || 
				(sym->get_opc().get_opc_state() == publish)) {
				int ret = process_type_tree (*sym, process, prefix);
				num += ret;
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
		if (!n.get_name().empty()) {
			return process_type_tree (symbol.get_type_name(), 
				symbol.get_type_decoration(), 
				symbol.get_opc(), symbol, process, n, 0);
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
		if ((typ.get_type_description() != arraytype) || 
			(typ.get_name().find ('[') == std::stringcase::npos)) {
				defopc.add (typ.get_opc());
		}
		switch (typ.get_type_description()) {
		case simple :
			// next level
			return process_type_tree (typ.get_name(), 
				typ.get_type_decoration(), defopc, loc, process, varname, level);
		case arraytype :
			// process array and iterate over all subindices
			return process_array (typ, typ.get_array_dimensions(), defopc, loc, 
				process, varname, level);
		case enumtype :
			if (get_process_tags() == process_atomic || 
				get_process_tags() == process_all) {
				// check if enum is contained with 0 to 15
				bool withinhex = true;
				for (enum_map::const_iterator e = typ.get_enum_list().begin(); 
					e != typ.get_enum_list().end(); ++e) {
						if ((e->first < 0) || (e->first >= 16)) {
							withinhex = false;
							break;
						}
				}
				// if not, treat as int
				if (!withinhex) {
					process_arg arg (loc, varname, pt_int, defopc, typ.get_name(), true);
					return process (arg) ? 1 : 0;
				}
				// add opc property for enum values
				else {
					for (enum_map::const_iterator e = typ.get_enum_list().begin(); 
						e != typ.get_enum_list().end(); ++e) {
							defopc.get_properties().insert (
								property_el (8510 + e->first, e->second));
					}
					process_arg arg (loc, varname, pt_enum, defopc, typ.get_name(), true);
					return process (arg) ? 1 : 0;
				}
			}
		case structtype :
		case functionblock :
			{
				// Call process for entire array (not an atomic type)
				int num = 0;
				if (get_process_tags() == process_structured || 
					get_process_tags() == process_all) {
					process_arg arg (loc, varname, pt_binary, defopc, typ.get_name(), false);
					num = process (arg) ? 1 : 0;
				}
				// iterate over all structure items
				for (item_list::const_iterator i = typ.get_struct_list().begin(); 
					i !=  typ.get_struct_list().end(); ++i) {
					// calculate sub element location
					memory_location el_loc = loc;
					if (!el_loc.set_section (*i)) {
						continue;
					}
					// Add a dot to the variable name
					ParseUtil::variable_name n (varname);
					n.append (i->get_name(), i->get_opc(), "."); 
					opc_list o (defopc);
					o.add (i->get_opc());
					num += process_type_tree (i->get_type_name(), 
						i->get_type_decoration(), o, el_loc, process, n, level + 1);
				}
				return num; 
			}
		default :
			fprintf (stderr, "Unknown type for %s\n", varname.get_name().c_str());
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
			(typ == "BYTE")  || (typ == "WORD") || (typ == "DWORD") || (typ == "LWORD")) {
			if (get_process_tags() == process_atomic || 
				get_process_tags() == process_all) {
				process_arg arg (loc, varname, pt_int, defopc, typ, true);
				return process (arg) ? 1 : 0;
			}
			else return 0;
		}
		else if (typ == "REAL" || typ == "LREAL") {
			if (get_process_tags() == process_atomic || 
				get_process_tags() == process_all) {
				process_arg arg (loc, varname, pt_real, defopc, typ, true);
				return process (arg) ? 1 : 0;
			}
			else return 0;
		}
		else if (typ == "BOOL") {
			if (get_process_tags() == process_atomic || 
				get_process_tags() == process_all) {
				process_arg arg (loc, varname, pt_bool, defopc, typ, true);
				return process (arg) ? 1 : 0;
			}
			else return 0;
		}
		else if (typ.compare (0, 6, "STRING") == 0) {
			if ((get_process_tags() == process_atomic || 
				 get_process_tags() == process_all) &&
				 !get_no_strings()) {
				process_arg arg (loc, varname, pt_string, defopc, typ, true);
				return process (arg) ? 1 : 0;
			}
			else return 0;
		}
		else if ((t = type_list.find (id, typ)) != nullptr) {
			return process_type_tree (*t, defopc, loc, process, varname, level);
		}
		else {
			fprintf (stderr, "Unknown type for %s\n", varname.get_name().c_str());
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
			dimension d = dim.front();
			dim.pop_front();
			// invalid number of elements
			if (d.second < 0) {
				fprintf (stderr, "Array with negative element number for %s\n", 
						 varname.get_name().c_str());
				return 0;
			}
			// Call process for entire array (not an atomic type)
			process_arg arg (loc, varname, pt_binary, defopc, typ.get_name(), false);
			int num = 0;
			if (get_process_tags() == process_structured || 
				get_process_tags() == process_all) {
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
			int el_bitsize = typ.get_bit_size() / d.second;
			for (int i = d.first; i < d.first + d.second; ++i) {
				memory_location el_loc = loc;
				bit_location el ((i - d.first) * el_bitsize, el_bitsize);
				if (!el_loc.set_section (el)) {
					continue;
				}
				char buf[40];
				sprintf_s (buf, sizeof (buf), "[%i]", i);
				ParseUtil::variable_name narr (varname);
				narr.append (buf, "");
				num += process_array (typ, dim, defopc, el_loc, process, narr, level);
			}
			return num;
		}
	}

}
