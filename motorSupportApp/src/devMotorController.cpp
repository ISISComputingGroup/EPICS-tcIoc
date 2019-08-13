#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <iocsh.h>
#include <epicsThread.h>

#include <asynOctetSyncIO.h>

#include "asynMotorController.h"
#include "asynMotorAxis.h"

#include <epicsExport.h>
#include "devMotor.h"

/** Creates a new devMotorController object.
  * \param[in] portName          The name of the asyn port that will be created for this driver
  * \param[in] MotorPortName     The name of the drvAsynSerialPort that was created previously to connect to the devMotor controller
  * \param[in] numAxes           The number of axes that this controller supports
  * \param[in] movingPollPeriod  The time between polls when any axis is moving
  * \param[in] idlePollPeriod    The time between polls when no axis is moving
  */
devMotorController::devMotorController(const char *portName, const char *MotorPortName, int numAxes, const char *pvPrefix)
  :  asynMotorController(portName, numAxes, NUM_VIRTUAL_MOTOR_PARAMS,
                         0, // No additional interfaces beyond those in base class
                         0, // No additional callback interfaces beyond those in base class
                         ASYN_CANBLOCK | ASYN_MULTIDEVICE,
                         1, // autoconnect
                         0, 0),  // Default priority and stack size
	pvPrefix(pvPrefix)
{
	printf("Created Controller\n");
	startPoller(1/1000.0, 1/1000.0, 2);
}


/** Creates a new devMotorController object.
  * Configuration command, called directly or from iocsh
  * \param[in] portName          The name of the asyn port that will be created for this driver
  * \param[in] MotorPortName  The name of the drvAsynIPPPort that was created previously to connect to the devMotor controller
  * \param[in] numAxes           The number of axes that this controller supports (0 is not used)
  */
extern "C" int devMotorCreateController(const char *portName, const char *MotorPortName, int numAxes, const char *pvPrefix)
{
	new devMotorController(portName, MotorPortName, 1+numAxes, pvPrefix);
	return(asynSuccess);
}

/** Reports on status of the driver
  * \param[in] fp The file pointer on which report information will be written
  * \param[in] level The level of report detail desired
  *
  * If details > 0 then information is printed about each axis.
  * After printing controller-specific information it calls asynMotorController::report()
  */
void devMotorController::report(FILE *fp, int level)
{
  fprintf(fp, "Twincat motor driver %s, numAxes=%d", this->portName, numAxes_);

  // Call the base class method
  asynMotorController::report(fp, level);
}

/** Code for iocsh registration */
static const iocshArg devMotorCreateControllerArg0 = {"Port name", iocshArgString};
static const iocshArg devMotorCreateControllerArg1 = {"EPICS ASYN TCP motor port name", iocshArgString};
static const iocshArg devMotorCreateControllerArg2 = {"Number of axes", iocshArgInt};
static const iocshArg devMotorCreateControllerArg3 = {"PV prefix", iocshArgString};
static const iocshArg * const devMotorCreateControllerArgs[] = {&devMotorCreateControllerArg0,
                                                             &devMotorCreateControllerArg1,
                                                             &devMotorCreateControllerArg2,
															 &devMotorCreateControllerArg3};
static const iocshFuncDef devMotorCreateControllerDef = {"devMotorCreateController", 4, devMotorCreateControllerArgs};
static void devMotorCreateContollerCallFunc(const iocshArgBuf *args)
{
  devMotorCreateController(args[0].sval, args[1].sval, args[2].ival, args[3].sval);
}


/* devMotorCreateAxis */
static const iocshArg devMotorCreateAxisArg0 = {"Controller port name", iocshArgString};
static const iocshArg devMotorCreateAxisArg1 = {"Axis number", iocshArgInt};
static const iocshArg * const devMotorCreateAxisArgs[] = {&devMotorCreateAxisArg0,
							      &devMotorCreateAxisArg1};
static const iocshFuncDef devMotorCreateAxisDef = {"devMotorCreateAxis", 2, devMotorCreateAxisArgs};
static void devMotorCreateAxisCallFunc(const iocshArgBuf *args)
{
  devMotorCreateAxis(args[0].sval, args[1].ival);
}

static void devMotorControllerRegister(void)
{
  iocshRegister(&devMotorCreateControllerDef, devMotorCreateContollerCallFunc);
  iocshRegister(&devMotorCreateAxisDef,       devMotorCreateAxisCallFunc);
}

extern "C" {
  epicsExportRegistrar(devMotorControllerRegister);
}
