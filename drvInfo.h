#pragma once
#include "stdafx.h"

/** @file drvInfo.h
	Header which includes classes for accessing Info data from EPICS. 
 ************************************************************************/

/** @namespace DevInfo
	DevInfo namespace
	@brief Namespace for info device support
 ************************************************************************/
namespace DevInfo {

/** Register Info commands to IOC shell
    This class registers the callback functions for the Info IOC commands
  */
class InfoRegisterToIocShell {
private:
	/// Constructor
    InfoRegisterToIocShell();
	/// Single static instance
	static InfoRegisterToIocShell gInfoRegisterToIocShell;
};

}