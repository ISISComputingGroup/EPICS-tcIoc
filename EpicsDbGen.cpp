#include "stdafx.h"
#include "ParseTpy.h"
#include "TpyToEpics.h"

using namespace std;
using namespace ParseUtil;
using namespace ParseTpy;
using namespace EpicsTpy;


/** @file EpicsDbGen.cpp
	Source for the main program that generates an EPICs .db file
 ************************************************************************/

/** Main program
 ************************************************************************/
int main(int argc, char *argv[])
{
	tpy_file		tpyfile;
	stringcase		inpfilename;
	stringcase		outfilename;
	stringcase		aliasname;
	bool			listing = false;
	bool			macros = false;
	bool			argp_list[100] = {false};
	bool			argp_db[100] = {false};
	bool			argp_macro[100] = {false};
	int				help = 0;

	// command line parsing
	if (argc > 100) argc = 100;
	for (int i = 1; i < argc; ++i) {
		stringcase arg (argv[i] ? argv[i] : 0);
		stringcase next (i + 1 < argc && argv[i+1] ? argv[i+1] : "");
		// specify input file
		if ((arg == "-i" || arg == "/i") && i + 1 < argc) {
			inpfilename = next;
			argp_list[i] = argp_db[i] = argp_macro[i] = true;
			i += 1;
			argp_list[i] = argp_db[i] = argp_macro[i] = true;
		}
		// specify output file
		else if ((arg == "-o" || arg == "/o") && i + 1 < argc) {
			outfilename = next;
			argp_list[i] = argp_db[i] = argp_macro[i] = true;
			i += 1;
			argp_list[i] = argp_db[i] = argp_macro[i] = true;
		}
		// specify alias name
		else if ((arg == "-a" || arg == "/a") && i + 1 < argc) {
			aliasname = next;
			argp_list[i] = argp_db[i] = argp_macro[i] = true;
			i += 1;
			argp_list[i] = argp_db[i] = argp_macro[i] = true;
		}
		// ask for help
		else if (arg == "-h" || arg == "/h" ) {
			help = 1;
			argp_list[i] = argp_db[i] = argp_macro[i] = true;
		}
		else if (arg.compare (0, 2, "/l") == 0 || 
				 arg.compare (0, 2, "-l") == 0) {
			listing = true;
			macros = false;
		}
		else if (arg.compare (0, 2, "/m") == 0 || 
				 arg.compare (0, 2, "-m") == 0) {
			macros = true;
			listing = false;
		}
	}
	bool* argp = macros ? argp_macro : (listing ? argp_list : argp_db);
	tpyfile.getopt (argc, argv, argp);
	// default conversion rules
	epics_list_processing	listproc;
	epics_db_processing		dbproc;
	epics_macrofiles_processing	macroproc;
	if (macros) {
		macroproc = epics_macrofiles_processing (aliasname, outfilename, false, argc, argv, argp_macro);
		macroproc.set_indirname (inpfilename);
	}
	else if (listing) {
		listproc = epics_list_processing (outfilename, argc, argv, argp_list);
	}
	else {
		dbproc = epics_db_processing (outfilename, argc, argv, argp_db);
	}

	// check if all arguments were processed
	for (int i = 1; i < argc; ++i) {
		if (!argp[i]) help = 2;
	}
	// Call help when asked, or when unprocessed options exit
	if (help) {
		printf ("Usage: EpicsDbGen ['options'] -i 'input' -o 'output'\n"
			"       Generates an EPICS database from a TwinCAT tpy file.\n"
			"       -ea exports all variables regardless of their opc setting\n"
			"       -l[l][a|e|b] generate an [extended] [atomic|epics|burt] channel listing\n"
			"       -m[f|e|a] generate a macro lists with fields|errors|all\n"
			"       -a 'alias' alias name for plc name\n"
			"       -r[n|d] no|dot conversion rule for EPICS names\n"
			"       -c[u|l] force upper/lower case for EPICS names\n" 
			"       -yd includes leading dot\n" 
			"       -devtc uses TwinCAT name in INPUT/OUTPUT fields instead of OPC\n"
			"       -ni replaces array brackets with underscore\n" 
			"       -ns ignores channels of type string\n"
			"       -sio splits database into input only and input/ouput recrods\n"
			"       -sn 'num' splits database into files with no more than num records\n"
			"       -i 'input' input file name (stdin when omitted)\n"
			"       -o 'output' output database file (stdout when omitted)\n");
		if (help == 2) return 1; 
		else return 0;
	}
	if (macros && !macroproc) {
		fprintf (stderr, "Failed to access directory %s and/or %s.\n", 
			macroproc.get_outdirname().c_str(), macroproc.get_indirname().c_str());
		return 1;
	}
	if ((listing && !listproc) || (!listing && !macros && !dbproc)) {
		fprintf (stderr, "Failed to open output %s.\n", outfilename.c_str());
		return 1;
	}

	// open input file
	FILE* inpf = stdin;
	if (!inpfilename.empty()) {
		#pragma warning (disable : 4996)
		inpf = fopen (inpfilename.c_str(), "r");
		#pragma warning (default : 4996)
		if (!inpf) {
			fprintf (stderr, "Failed to open input %s.\n", inpfilename.c_str());
			return 1;
		}
	}
	fprintf (stderr, "\nInput from %s\n", 
		inpf == stdin ? "stdin" : inpfilename.c_str());

	// print status of output
	fprintf (stderr, "Output to %s\n", 
		listproc.get_file() == stdout ? "stdout" : outfilename.c_str());
	fprintf (stderr, "Arguments are");
	// print arguments
	for (int i = 1; i < argc; ++i) {
		stringcase arg (argv[i] ? argv[i] : 0);
		if ((arg == "-i") || (arg == "/i") || 
			(arg == "-o") || (arg == "/o")) {
				++i;
				continue;
		}
		fprintf (stderr, " %s", arg.c_str());
	}
	fprintf (stderr, "\n");

	// Parse input
	if (!tpyfile.parse (inpf)) {
		fprintf (stderr, "Unable to parse %s.\n", inpfilename.c_str());
		return 1;
	}

	// generate macro files
	if (macros) {
		macroproc.set_twincat3 (tpyfile.get_project_info().get_tcat_version_major() >= 3);
		if (!tpyfile.process_symbols (macroproc)) {
			fprintf (stderr, "Unable to generate listing.\n");
			return 1;
		}
		macroproc.flush();
	}
	// generate listing
	else if (listing) {
		if (!tpyfile.process_symbols (listproc)) {
			fprintf (stderr, "Unable to generate listing.\n");
			return 1;
		}
		listproc.flush();
	}
	// generating epics db
	else {
		if (!tpyfile.process_symbols (dbproc)) {
			fprintf (stderr, "Unable to generate record database.\n");
			return 1;
		}
		dbproc.flush();
	}

	// write summary information
	if (macros) {
		fprintf (stdout, "\nSummary:\n");
		fprintf (stdout, "Total number of processed records = %5d\n", 
			macroproc.get_processed_total());
		fprintf (stdout, "Total number files read           = %5d\n",  
			macroproc.get_filein_total());
		fprintf (stdout, "Total number files written        = %5d\n",  
			macroproc.get_fileout_total());
	}
	else {
		split_io_support* summary= listing ? 
			(split_io_support*)(&listproc) : (split_io_support*)(&dbproc);
		fprintf (outfilename.empty() ? stdout : stderr, "\nSummary:\n");
		fprintf (outfilename.empty() ? stdout : stderr, 
			"Total number of processed records = %5d\n", 
			summary->get_processed_total());
		fprintf (outfilename.empty() ? stdout : stderr, 
			"Total number of input records     = %5d\n", 
			summary->get_processed_readonly());
		fprintf (outfilename.empty() ? stdout : stderr, 
			"Total number of in/out records    = %5d\n",  
			summary->get_processed_io());
	}

	return 0;
}
