#pragma once
#include "stdafx.h"

/** @file ParseUtilConst.h
	Header which includes OPC constants. 
 ************************************************************************/

/** @namespace ParseUtil
	ParseUtil name space 
	@brief Namespace for parsing utilities
 ************************************************************************/
namespace ParseTpy {

/** @defgroup parsetpyconstxml  XML tpy file constants
 ************************************************************************/
/** @{ */

const char* const xmlPlcProjectInfo = "PlcProjectInfo";
const char* const xmlProjectInfo = "ProjectInfo";
const char* const xmlRoutingInfo = "RoutingInfo";
const char* const xmlCompilerInfo = "CompilerInfo";
const char* const xmlAdsInfo = "AdsInfo";
const char* const xmlDataTypes = "DataTypes";
const char* const xmlDataType = "DataType";
const char* const xmlSymbols = "Symbols";
const char* const xmlSymbol = "Symbol";
const char* const xmlProperties = "Properties";
const char* const xmlProperty = "Property";

const char* const xmlCompilerVersion = "CompilerVersion";
const char* const xmlTwinCATVersion = "TwinCATVersion";
const char* const xmlCpuFamily = "CpuFamily";
const char* const xmlNetId = "NetId";
const char* const xmlPort = "Port";
const char* const xmlTargetName = "TargetName";

const char* const xmlName = "Name";
const char* const xmlType = "Type";
const char* const xmlAttrDecoration = "Decoration";
const char* const xmlAttrPointer = "Pointer";
const char* const xmlIGroup = "IGroup";
const char* const xmlIOffset = "IOffset";
const char* const xmlBitSize = "BitSize";
const char* const xmlBitOffs = "BitOffs";
const char* const xmlArrayInfo = "ArrayInfo";
const char* const xmlArrayLBound = "LBound";
const char* const xmlArrayElements = "Elements";
const char* const xmlSubItem = "SubItem";
const char* const xmlFbInfo = "FbInfo";
const char* const xmlEnumInfo = "EnumInfo";
const char* const xmlEnumText = "Text";
const char* const xmlEnumEnum = "Enum";
const char* const xmlEnumComment = "Comment";

const char* const xmlValue = "Value";
const char* const xmlDesc = "Desc";
/** @} */

/** @defgroup parsetpyconstopc  OPC tpy file constants
 ************************************************************************/
/** @{ */

const char* const opcExport = "opc";
const char* const opcProp = "opc_prop";
const char* const opcBracket = "[";
/** @} */

}