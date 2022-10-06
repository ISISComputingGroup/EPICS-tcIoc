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

constexpr int OPC_PROP_CDT=       1;	/**< canonocal data type */
constexpr int OPC_PROP_VALUE=     2;	/**< value */
constexpr int OPC_PROP_QUALITY=   3;	/**< data quality flag */
constexpr int OPC_PROP_TIME=      4;	/**< timestamp */
constexpr int OPC_PROP_RIGHTS=    5;	/**< access right: 1 read, 2 write, 3 read/write */
constexpr int OPC_PROP_SCANRATE=  6;	/**< scan rate */

constexpr int OPC_PROP_UNIT=      100;	/**< unit string -> EGU */
constexpr int OPC_PROP_DESC=      101;	/**< description string -> DESC */
constexpr int OPC_PROP_HIEU=      102;	/**< high expectation value -> HOPR */
constexpr int OPC_PROP_LOEU=      103;	/**< low expectation value -> LOPR */
constexpr int OPC_PROP_HIRANGE=   104;	/**< absolute maximum value -> DRVH */
constexpr int OPC_PROP_LORANGE=   105;	/**< absolute minumum value -> DRVL */
constexpr int OPC_PROP_CLOSE=     106;	/**< label for close state -> ONAM */
constexpr int OPC_PROP_OPEN=      107;	/**< label for open state -> ZNAM */
constexpr int OPC_PROP_TIMEZONE=  108;	/**< time zone */

constexpr int OPC_PROP_FGC=       201;	/**< foreground color */
constexpr int OPC_PROP_BGC=       202;	/**< background color */
constexpr int OPC_PROP_BLINK=     203;	/**< blinking */
constexpr int OPC_PROP_BMP=       204;	/**< bmp file */
constexpr int OPC_PROP_SND=       205;	/**< sound file */
constexpr int OPC_PROP_HTML=      206;	/**< html file */
constexpr int OPC_PROP_AVI=       207;	/**< avi file */

constexpr int OPC_PROP_ALMSTAT=   300;	/**< status */
constexpr int OPC_PROP_ALMHELP=   301;	/**< help */
constexpr int OPC_PROP_ALMAREAS=  302;	/**< area */
constexpr int OPC_PROP_ALMPRIMARYAREA= 303;	/**< primery area */
constexpr int OPC_PROP_ALMCONDITION=   304;	/**< condition */
constexpr int OPC_PROP_ALMLIMIT=  305;	/**< limit */
constexpr int OPC_PROP_ALMDB=     306;	/**< dead band, tolerance */
constexpr int OPC_PROP_ALMHH=     307;	/**< high high alarm -> HIHI */
constexpr int OPC_PROP_ALMH=      308;	/**< high alarm -> HIGH */
constexpr int OPC_PROP_ALML=      309;	/**< low alarm -> LOW */
constexpr int OPC_PROP_ALMLL=     310;	/**< low low alaam -> LOLO */
constexpr int OPC_PROP_ALMROC=    311;	/**< rate of change */
constexpr int OPC_PROP_ALMDEV=    312;	/**< deviation */

constexpr int OPC_PROP_PREC=	  8500;	/**< precision */
constexpr int OPC_PROP_ZRST=	  8510;	/**< zero string ... */
constexpr int OPC_PROP_FFST=	  8525; /**< ... fifteen string */
constexpr int OPC_PROP_RECTYPE=	  8600;	/**< record type */
constexpr int OPC_PROP_INOUT=	  8601;	/**< input or output */
const char* const OPC_PROP_INPUT = "input"; /**< input */
const char* const OPC_PROP_OUTPUT = "output"; /**< output */
constexpr int OPC_PROP_TSE=		  8602;	/**< time stamp */
constexpr int OPC_PROP_PINI=	  8603;	/**< initialization */
constexpr int OPC_PROP_DTYP=	  8604;	/**< DTYP field: opc or opcRaw */
constexpr int OPC_PROP_SERVER=	  8610;	/**< server name */
constexpr int OPC_PROP_PLCNAME=   8611; /**< tc name including ads routing info and port */
constexpr int OPC_PROP_ALIAS=     8620; /**< alias for structure item or symbol name */
const char* const OPC_NAME_ALIAS = "ALIAS"; /**< alias name */
constexpr int OPC_PROP_ALMOSV=	  8700;	/**< alarm: one severity */
constexpr int OPC_PROP_ALMZSV=	  8701;	/**< alarm: zero severity */
constexpr int OPC_PROP_ALMCOSV=	  8702;	/**< alarm: change of state severity */
constexpr int OPC_PROP_ALMUNSV=	  8703;	/**< alarm: unknown state severity */
constexpr int OPC_PROP_ALMZRSV=	  8710;	/**< alarm: one state severity */
constexpr int OPC_PROP_ALMFFSV=	  8725;	/**< alarm: fifteen state severity */
constexpr int OPC_PROP_ALMHHSV=	  8727;	/**< alarm: hihi severity */
constexpr int OPC_PROP_ALMHSV=	  8728;	/**< alarm: high severity */
constexpr int OPC_PROP_ALMLSV=	  8729;	/**< alarm: low severity */
constexpr int OPC_PROP_ALMLLSV=	  8730;	/**< alarm: lolo severity */
constexpr int OPC_PROP_FIELD_BEG= 8800;	/**< Beginning of field,value combination */
constexpr int OPC_PROP_FIELD_END= 9000;	/**< End of field,value combinations */

/** @} */

}