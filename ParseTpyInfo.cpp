#include "stdafx.h"
#include <epicsStdio.h>
#include "ParseTpy.h"

using namespace std;
using namespace ParseTpy;
using namespace ParseUtil;

#ifdef _WIN32
#pragma warning (disable: 4996)
#endif

/** @file ParseTpyInfo.cpp
	Source file for tpy parsing methods.
 ************************************************************************/

/** Symbol processing
    @brief Symbol processing
 ************************************************************************/
class syminfo_processing {
public:
	/// Constructor
	explicit syminfo_processing (FILE* outfile = 0, bool atomic = true)
		: outf (outfile ? outfile : stdout), firstline (true) {}
	/// Process
	bool operator() (const process_arg& arg);
protected:
	/// Ouptut file
	FILE*		outf;
	/// Firstline?
	bool		firstline;
};

bool syminfo_processing::operator() (const process_arg& arg)
{
	if (firstline) {
		firstline = false;
		fprintf (outf, " Basic    IGroup IOffset    Size Type                    Name\n");
	}
	// atomic?
	if (arg.is_atomic()) {
		fprintf (outf, " ");
	}
	else {
		fprintf (outf, "+");
	}
	// Print type (7 chars)
	switch (arg.get_process_type()) 
	{
	// Numeral type
	case pt_int:
		fprintf (outf, "int    ");
		break;
	// Floating point type
	case pt_real:
		fprintf (outf, "real   ");
		break;
	// Logic type
	case pt_bool:
		fprintf (outf, "bool   ");
		break;
	// String type
	case pt_string:
		fprintf (outf, "string ");
		break;
	// Enumerated type
	case pt_enum:
		fprintf (outf, "enum   ");
		break;
	// Binary type
	case pt_binary:
		fprintf (outf, "binary ");
		break;
	// Invalid type
	case pt_invalid:
	default:
		fprintf (outf, "?      ");
		break;
	}

	// Print memory start address and size (24 chars)
	const process_arg_tc* parg = dynamic_cast<const process_arg_tc*>(&arg);
	if (parg) {
		fprintf(outf, " %7d %7d %7d", parg->get_igroup(), parg->get_ioffset(), (int)(parg->get_bytesize()));
	}

	// Print type and varname
	fprintf (outf, " %-23s %-s\n", arg.get_type_name().c_str(), arg.get_name().c_str()); 

	return true;
}


/** Main program for tpyinfo
    @brief tpyinfo
 ************************************************************************/
int main (int argc, char *argv[])
{
	int			help = 0;
	stringcase	prefix;
	stringcase	inpfilename;
	stringcase	outfilename;
	tpy_file	tpyfile;
	bool		argp[100] = {false};

	// command line parsing
	if (argc > 100) argc = 100;
	tpyfile.getopt (argc, argv, argp);
	for (int i = 1; i < argc; ++i) {
		stringcase arg (argv[i] ? argv[i] : "");
		stringcase next (i + 1 < argc && argv[i+1] ? argv[i+1] : "");
		if (arg == "-h" || arg == "/h") {
			help = 1;
		}
		else if ((arg == "-p" || arg == "/p") && i + 1 < argc) {
			prefix = next;
			i += 1;
		}
		else if ((arg == "-i" || arg == "/i") && i + 1 < argc) {
			inpfilename = next;
			i += 1;
		}
		else if ((arg == "-o" || arg == "/o") && i + 1 < argc) {
			outfilename = next;
			i += 1;
		}
		else {
			if (!argp[i]) help = 2;
		}
	}
	if (help) {
		printf ("Usage: tpyinfo [opt] [-p 'prefix'] -i 'tpyfile' -o 'outfile'\n"
			"       Displays information about the individual symbols of a TwinCAT tpy file.\n"
			"       -ea includes all symbols rather than the ones available by opc\n"
			"       -ps only output atomic types\n"
			"       -pc only output structured types\n"
			"       -ns suppress string variables\n"
			"       -p 'prefix' Add a prefix to the variable name\n"
			"       -i 'tpyfile' input file name (stdin when omitted)\n"
		    "       -o 'outfile' output listing (stdout when omitted)\n");
		if (help == 2) return 1; else return 0;
	}

	// open input file
	FILE* inpf = stdin;
	FILE* outf = stdout;
	if (!inpfilename.empty()) {
		inpf = fopen (inpfilename.c_str(), "r");
		if (!inpf) {
			fprintf (stderr, "Failed to open input %s.\n", inpfilename.c_str());
			return 1;
		}
	}
	// open output file
	if (!outfilename.empty()) {
		outf = fopen (outfilename.c_str(), "w");
		if (!outf) {
			fprintf (stderr, "Failed to open output %s.\n", outfilename.c_str());
			return 1;
		}
	}

	// parse tpy file
	clock_t t1 = clock();
	if (!tpyfile.parse (inpf)) {
		fprintf (stderr, "Unable to parse %s\n", inpfilename.c_str());
		return 1;
	}

	// work through the symbol list
	clock_t t2 = clock();
        auto outfsym = syminfo_processing (outf);
	if (!tpyfile.process_symbols (outfsym, prefix)) {
		fprintf (stderr, "Unable to parse %s\n", inpfilename.c_str());
		return 1;
	}
	clock_t t3 = clock();
	fprintf (stderr, "Time to parse file %g sec, time to build list %g sec.\n", 
		static_cast<double>((int64_t)t2 - (int64_t)t1)/CLOCKS_PER_SEC, 
		static_cast<double>((int64_t)t3 - (int64_t)t2)/CLOCKS_PER_SEC);

	// close files
	if (!inpfilename.empty()) {
		fclose (inpf);
	}
	if (!outfilename.empty()) {
		fclose (outf);
	}
	return 0;
}
