/*
FILENAME... devMotorController.cpp
*/

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
devMotorController::devMotorController(const char *portName, const char *MotorPortName, int numAxes)
  :  asynMotorController(portName, numAxes, NUM_VIRTUAL_MOTOR_PARAMS,
                         0, // No additional interfaces beyond those in base class
                         0, // No additional callback interfaces beyond those in base class
                         ASYN_CANBLOCK | ASYN_MULTIDEVICE,
                         1, // autoconnect
                         0, 0)  // Default priority and stack size
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
extern "C" int devMotorCreateController(const char *portName, const char *MotorPortName, int numAxes)
{
  new devMotorController(portName, MotorPortName, 1+numAxes);
  return(asynSuccess);
}

/** Writes a string to the controller and reads a response.
  * Disconnects in case of error
  */
asynStatus devMotorController::writeReadOnErrorDisconnect(void)
{
  asynStatus status = asynSuccess;
  return status;
}

void devMotorController::handleStatusChange(asynStatus status)
{
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

/** Returns a pointer to an devMotorAxis object.
  * Returns NULL if the axis number encoded in pasynUser is invalid.
  * \param[in] pasynUser asynUser structure that encodes the axis index number. */
devMotorAxis* devMotorController::getAxis(asynUser *pasynUser)
{
  return static_cast<devMotorAxis*>(asynMotorController::getAxis(pasynUser));
}

/** Returns a pointer to an devMotorAxis object.
  * Returns NULL if the axis number encoded in pasynUser is invalid.
  * \param[in] axisNo Axis index number. */
devMotorAxis* devMotorController::getAxis(int axisNo)
{
  return static_cast<devMotorAxis*>(asynMotorController::getAxis(axisNo));
}


asynStatus devMotorController::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
  int function = pasynUser->reason;
  devMotorAxis *pAxis;
  pAxis = getAxis(pasynUser);
  if (!pAxis) return asynError;

  (void)pAxis->setIntegerParam(function, value);
  return asynSuccess;
}

/** Code for iocsh registration */
static const iocshArg devMotorCreateControllerArg0 = {"Port name", iocshArgString};
static const iocshArg devMotorCreateControllerArg1 = {"EPICS ASYN TCP motor port name", iocshArgString};
static const iocshArg devMotorCreateControllerArg2 = {"Number of axes", iocshArgInt};
static const iocshArg * const devMotorCreateControllerArgs[] = {&devMotorCreateControllerArg0,
                                                             &devMotorCreateControllerArg1,
                                                             &devMotorCreateControllerArg2};
static const iocshFuncDef devMotorCreateControllerDef = {"devMotorCreateController", 3, devMotorCreateControllerArgs};
static void devMotorCreateContollerCallFunc(const iocshArgBuf *args)
{
  devMotorCreateController(args[0].sval, args[1].sval, args[2].ival);
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
