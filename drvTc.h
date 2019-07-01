#pragma once
#include "stdafx.h"

/** @file drvTc.h
	Header which includes classes for accessing TwinCAT/ADS from EPICS. 
 ************************************************************************/

/** @namespace DevTc
	DevTc namespace
	@brief Namespace for TCat device support
 ************************************************************************/
namespace DevTc {

/** Register TC commands to IOC shell
    This class registers the callback functions for the TC IOC commands
	@brief Register TC commands
  */
class tcRegisterToIocShell {
private:
	/// Constructor
    tcRegisterToIocShell();
	/// Single static instance
	static tcRegisterToIocShell gtcRegisterToIocShell;
};

}