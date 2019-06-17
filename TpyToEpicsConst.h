#pragma once
#include "stdafx.h"

/** @file TpyToEpicsConst.h
	Header which includes EPICS constants needed to generate an EPICS db
	file fromt a parsed TwinCAT tpy file. 
 ************************************************************************/

/** @namespace ParseTpy
	ParseTpy name space 
 ************************************************************************/
namespace EpicsTpy {

/** @defgroup tpytoepicsconstmax EPICS constants
 ************************************************************************/
/** @{ */

const int MAX_EPICS_CHANNEL=	54;	/**< maximum length of EPICS channel name */
const int MAX_EPICS_DESC=		40;	/**< maximum length of EPICS channel description */
const int MAX_EPICS_STRING=		40;	/**< maximum length of EPICS strings */
const int MAX_EPICS_UNIT=		15;	/**< maximum length of EPICS channel unit string */

const char* const EPICS_DB_EGU=     "EGU";	/**< unit string */
const char* const EPICS_DB_DESC=    "DESC";	/**< description string */
const char* const EPICS_DB_HOPR=    "HOPR";	/**< high ops value string */
const char* const EPICS_DB_LOPR=    "LOPR";	/**< low ops value string */
const char* const EPICS_DB_DRVH=    "DRVH";	/**< drive high string */
const char* const EPICS_DB_DRVL=    "DRVL";	/**< drive low string */
const char* const EPICS_DB_ONAM=    "ONAM";	/**< one name string */
const char* const EPICS_DB_ZNAM=    "ZNAM";	/**< zero name string */

const char* const EPICS_DB_PREC=	"PREC";	/**< precision */
const char* const EPICS_DB_ZRST[16] = {"ZRST", "ONST", "TWST", "THST", "FRST", "FVST", "SXST", "SVST",
									   "EIST", "NIST", "TEST", "ELST", "TVST", "TTST", "FTST", "FFST"};	/**< enum string */
const char* const EPICS_DB_ZRVL[16] = {"ZRVL", "ONVL", "TWVL", "THVL", "FRVL", "FVVL", "SXVL", "SVVL", 
									   "EIVL", "NIVL", "TEVL", "ELVL", "TVVL", "TTVL", "FTVL", "FFVL"};	/**< enum val */

const char* const EPICS_DB_SCAN=	"SCAN";	/**< Scan */
const char* const EPICS_DB_INP=		"INP";	/**< input link */
const char* const EPICS_DB_OUT=		"OUT";	/**< output link */
const char* const EPICS_DB_TSE=		"TSE";	/**< time stamp */
const char* const EPICS_DB_PINI=	"PINI";	/**< initialization */
const char* const EPICS_DB_DTYP=	"DTYP";	/**< data type */

const char* const EPICS_DB_OSV=		"OSV";	/**< one severity */
const char* const EPICS_DB_ZSV=		"ZSV";	/**< zero severity */
const char* const EPICS_DB_COSV=	"COSV";	/**< change severity */

const char* const EPICS_DB_HIHI=	"HIHI";	/**< high severity high limit  */
const char* const EPICS_DB_HIGH=	"HIGH";	/**< low severity high limit  */
const char* const EPICS_DB_LOW=		"LOW";	/**< low severity low limit  */
const char* const EPICS_DB_LOLO=	"LOLO";	/**< high severity low limit  */
const char* const EPICS_DB_HYST=	"HYST";	/**< deadband  */
const char* const EPICS_DB_HHSV=	"HHSV";	/**< high severity high limit  */
const char* const EPICS_DB_HSV=		"HSV";	/**< low severity high limit  */
const char* const EPICS_DB_LSV=		"LSV";	/**< low severity low limit  */
const char* const EPICS_DB_LLSV=	"LLSV";	/**< high severity low limit  */

const char* const EPICS_DB_NOALARM=	"NO_ALARM"; /**< no alarm  */
const char* const EPICS_DB_MINOR=	"MINOR"; /**< minor alarm  */
const char* const EPICS_DB_MAJOR=	"MAJOR"; /**< major alarm  */

const char* const EPICS_DB_UNSV=	"UNSV";	/**< unknown severity */
const char* const EPICS_DB_ZRSV[16] = {"ZRSV", "ONSV", "TWSV", "THSV", "FRSV", "FVSV", "SXSV", "SVSV", 
									   "EISV", "NISV", "TESV", "ELSV", "TVSV", "TTSV", "FTSV", "FFSV"}; /**< enum alarm severity */

/** Names of forbidden EPICS record fields
 ************************************************************************/
const char* const EPICS_DB_FORBIDDEN[] = {"", EPICS_DB_SCAN, "DOL", EPICS_DB_INP, EPICS_DB_OUT, "VAL", "RVAL", "INIT", 
								 "ZRVL", "ONVL", "TWVL", "THVL", "FRVL", "FVVL", "SXVL", "SVVL", 
								 "EIVL", "NIVL", "TEVL", "ELVL", "TVVL", "TTVL", "FTVL", "FFVL", NULL};

/** Names of allowed EPICS record fields
 ************************************************************************/
const char* const EPICS_DB_ALLOWED[] =	{"LINR", "EGUF", "EGUL", "AOFF", "ASLO", "ESLO", "SMOO", "HIHI", "LOLO", "HIGH", "LOW", "HHSV", "LLSV", 
							 "HSV", "LSV", "HYST", "ZSV", "OSV", "COSV", "IVOA", "IVOV", "UNSV", "ZRSV",  "ONSV", "TWSV", "THSV",
							 "FRSV", "FVSV", "SXSV", "SVSV", "EISV", "NISV", "TESV", "ELSV", "TVSV", "TTSV", "FTSV", "FFSV", NULL};

/** Names of EPICS record fields which are numeric
 ************************************************************************/
const char* const EPICS_DB_NUMVAL[] = {EPICS_DB_TSE, EPICS_DB_HOPR, EPICS_DB_LOPR, EPICS_DB_DRVH, EPICS_DB_DRVL, "EGUF", "EGUL",
							 "AOFF", "ASLO", "ESLO", "SMOO", "PREC", "HIHI", "LOLO", "HIGH", "LOW", "HYST",
                             "ZRVL", "ONVL", "TWVL", "THVL", "FRVL", "FVVL", "SXVL", "SVVL", "EIVL", "NIVL", "TEVL", "ELVL",
							 "TVVL", "TTVL", "FTVL", "FFVL", NULL};


/** DAQ datatype name */
const char* const LIGODAQ_DATATYPE_NAME = "datatype";
const int LIGODAQ_DATATYPE_FLOAT = 4;	/**< DAQ datatype is float */
const int LIGODAQ_DATATYPE_INT32 = 2;	/**< DAQ datatype is int32 */
const int LIGODAQ_DATATYPE_DEFAULT = LIGODAQ_DATATYPE_FLOAT;	/**< default DAQ datatype is float */

const char* const LIGODAQ_UNIT_NAME = "units";	/**< DAQ unit name */
const char* const LIGODAQ_UNIT_NONE = "none";	/**<  DAQ unit for no unit */
const char* const LIGODAQ_UNIT_DEFAULT = LIGODAQ_UNIT_NONE;	/**< default DAQ unit is none */

/** DAQ ini file header: substitute defaults */
const char* const LIGODAQ_INI_HEADER = 
"[default]\n"
"gain=1.00\n"
"datatype=%i\n"
"ifoid=0\n"
"slope=6.1028e-05\n"
"acquire=3\n"
"offset=0\n"
"units=%s\n"
"dcuid=4\n"
"datarate=16";

/** @} */
}
