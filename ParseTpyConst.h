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

/// PLC project info
const char* const xmlPlcProjectInfo = "PlcProjectInfo";
/// Project info
const char* const xmlProjectInfo = "ProjectInfo";
/// Routing info
const char* const xmlRoutingInfo = "RoutingInfo";
/// Compiler info
const char* const xmlCompilerInfo = "CompilerInfo";
/// ADS info
const char* const xmlAdsInfo = "AdsInfo";
/// Data types
const char* const xmlDataTypes = "DataTypes";
/// Data type
const char* const xmlDataType = "DataType";
/// Symbols
const char* const xmlSymbols = "Symbols";
/// Symbol
const char* const xmlSymbol = "Symbol";
/// Properties
const char* const xmlProperties = "Properties";
/// Property
const char* const xmlProperty = "Property";

/// Compiler version
const char* const xmlCompilerVersion = "CompilerVersion";
/// TwinCAT version
const char* const xmlTwinCATVersion = "TwinCATVersion";
///CPU family
const char* const xmlCpuFamily = "CpuFamily";
/// Net ID
const char* const xmlNetId = "NetId";
/// Port
const char* const xmlPort = "Port";
/// Target name
const char* const xmlTargetName = "TargetName";

/// Name
const char* const xmlName = "Name";
/// Type
const char* const xmlType = "Type";
/// Decoration
const char* const xmlAttrDecoration = "Decoration";
/// Pointer
const char* const xmlAttrPointer = "Pointer";
/// I Group
const char* const xmlIGroup = "IGroup";
/// I Offset
const char* const xmlIOffset = "IOffset";
/// Bit size
const char* const xmlBitSize = "BitSize";
/// Bit Offset
const char* const xmlBitOffs = "BitOffs";
/// Array info
const char* const xmlArrayInfo = "ArrayInfo";
/// Lower bound
const char* const xmlArrayLBound = "LBound";
/// Elements
const char* const xmlArrayElements = "Elements";
/// Sub item
const char* const xmlSubItem = "SubItem";
/// Fb info
const char* const xmlFbInfo = "FbInfo";
/// Enum info
const char* const xmlEnumInfo = "EnumInfo";
/// Text
const char* const xmlEnumText = "Text";
/// Enum
const char* const xmlEnumEnum = "Enum";
/// Comment
const char* const xmlEnumComment = "Comment";

/// Value
const char* const xmlValue = "Value";
/// Description
const char* const xmlDesc = "Desc";


/// OPC
const char* const opcExport = "opc";
/// OPC property
const char* const opcProp = "opc_prop";
/// OPC bracket
const char* const opcBracket = "[";

/// Substitution 
const char* const xmlSubstitution = "TcSubstitution";
/// Substitution EPICS channel name 
const char* const xmlAlias = "Alias";
/** @} */

}