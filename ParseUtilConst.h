#pragma once
#include "stdafx.h"

/** @file ParseTpyConst.h
	Header which includes constants needed to parse a TwinCAT tpy file. 
 ************************************************************************/

/** @namespace ParseTpy
	ParseTpy name space 
	@brief Namespace for tpy parsing
 ************************************************************************/
namespace ParseUtil {

/** @defgroup parsetpyconstopcprop OPC property constants
 ************************************************************************/
/** @{ */

const int OPC_PROP_CDT=       1;	/* canonocal data type */
const int OPC_PROP_VALUE=     2;	/* value */
const int OPC_PROP_QUALITY=   3;	/* data quality flag */
const int OPC_PROP_TIME=      4;	/* timestamp */
const int OPC_PROP_RIGHTS=    5;	/* access right: 1 read, 2 write, 3 read/write */
const int OPC_PROP_SCANRATE=  6;	/* scan rate */

const int OPC_PROP_UNIT=      100;	/* unit string -> EGU */
const int OPC_PROP_DESC=      101;	/* description string -> DESC */
const int OPC_PROP_HIEU=      102;	/* high expectation value -> HOPR */
const int OPC_PROP_LOEU=      103;	/* low expectation value -> LOPR */
const int OPC_PROP_HIRANGE=   104;	/* absolute maximum value -> DRVH */
const int OPC_PROP_LORANGE=   105;	/* absolute minumum value -> DRVL */
const int OPC_PROP_CLOSE=     106;	/* label for close state -> ONAM */
const int OPC_PROP_OPEN=      107;	/* label for open state -> ZNAM */
const int OPC_PROP_TIMEZONE=  108;	/* time zone */

const int OPC_PROP_FGC=       201;	/* foreground color */
const int OPC_PROP_BGC=       202;	/* background color */
const int OPC_PROP_BLINK=     203;	/* blinking */
const int OPC_PROP_BMP=       204;	/* bmp file */
const int OPC_PROP_SND=       205;	/* sound file */
const int OPC_PROP_HTML=      206;	/* html file */
const int OPC_PROP_AVI=       207;	/* avi file */

const int OPC_PROP_ALMSTAT=   300;	/* status */
const int OPC_PROP_ALMHELP=   301;	/* help */
const int OPC_PROP_ALMAREAS=  302;	/* area */
const int OPC_PROP_ALMPRIMARYAREA= 303;	/* primery area */
const int OPC_PROP_ALMCONDITION=   304;	/* condition */
const int OPC_PROP_ALMLIMIT=  305;	/* limit */
const int OPC_PROP_ALMDB=     306;	/* dead band, tolerance */
const int OPC_PROP_ALMHH=     307;	/* high high alarm -> HIHI */
const int OPC_PROP_ALMH=      308;	/* high alarm -> HIGH */
const int OPC_PROP_ALML=      309;	/* low alarm -> LOW */
const int OPC_PROP_ALMLL=     310;	/* low low alaam -> LOLO */
const int OPC_PROP_ALMROC=    311;	/* rate of change */
const int OPC_PROP_ALMDEV=    312;	/* deviation */

const int OPC_PROP_PREC=	  8500;	/* precision */
const int OPC_PROP_ZRST=	  8510;	/* zero string ... */
const int OPC_PROP_FFST=	  8525; /* ... fifteen string */
const int OPC_PROP_RECTYPE=	  8600;	/* record type */
const int OPC_PROP_INOUT=	  8601;	/* input or output */
const char* const OPC_PROP_INPUT = "input";
const char* const OPC_PROP_OUTPUT = "output";
const int OPC_PROP_TSE=		  8602;	/* time stamp */
const int OPC_PROP_PINI=	  8603;	/* initialization */
const int OPC_PROP_DTYP=	  8604;	/* DTYP field: opc or opcRaw */
const int OPC_PROP_SERVER=	  8610;	/* server name */
const int OPC_PROP_PLCNAME=   8611; /* tc name including ads routing info and port */
const int OPC_PROP_ALIAS=     8620; /* alias for structure item or symbol name */
const int OPC_PROP_ALMOSV=	  8700;	/* alarm: one severity */
const int OPC_PROP_ALMZSV=	  8701;	/* alarm: zero severity */
const int OPC_PROP_ALMCOSV=	  8702;	/* alarm: change of state severity */
const int OPC_PROP_ALMUNSV=	  8703;	/* alarm: unknown state severity */
const int OPC_PROP_ALMZRSV=	  8710;	/* alarm: one state severity */
const int OPC_PROP_ALMFFSV=	  8725;	/* alarm: fifteen state severity */
const int OPC_PROP_ALMHHSV=	  8727;	/* alarm: hihi severity */
const int OPC_PROP_ALMHSV=	  8728;	/* alarm: high severity */
const int OPC_PROP_ALMLSV=	  8729;	/* alarm: low severity */
const int OPC_PROP_ALMLLSV=	  8730;	/* alarm: lolo severity */

/** @} */

}