/*
FILENAME... devMotorAxis.cpp
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dbAccess.h>
#include <stdexcept>
#include <epicsThread.h>

#include "asynMotorController.h"
#include "asynMotorAxis.h"
#include <errlog.h>
#include <epicsExport.h>
#include "devMotor.h"

#ifndef ASYN_TRACE_INFO
#define ASYN_TRACE_INFO      0x0040
#endif

#ifndef MIN
#   define MIN(x,y)  (((x) < (y)) ? (x) : (y))
#endif

//
// These are the devMotorAxis methods
//

/** Creates a new devMotorAxis object.
  * \param[in] pC Pointer to the EssMCAGmotorController to which this axis belongs.
  * \param[in] axisNo Index number of this axis, range 1 to pC->numAxes_. (0 is not used)
  *
  *
  * Initializes register numbers, etc.
  */
devMotorAxis::devMotorAxis(devMotorController *pC, int axisNo)
  : asynMotorAxis(pC, axisNo),
    pC_(pC)
{
  printf("Axis created\n");
  pC_->wakeupPoller();
}


extern "C" int devMotorCreateAxis(const char *devMotorName, int axisNo)
{
  devMotorController *pC;

  pC = (devMotorController*) findAsynPortDriver(devMotorName);
  if (!pC)
  {
    printf("Error port %s not found\n", devMotorName);
    return asynError;
  }
  pC->lock();
  new devMotorAxis(pC, axisNo);
  pC->unlock();
  return asynSuccess;
}

/** Move the axis to a position, either absolute or relative
  * \param[in] position in mm
  * \param[in] relative (0=absolute, otherwise relative)
  * \param[in] minimum velocity, mm/sec
  * \param[in] maximum velocity, mm/sec
  * \param[in] acceleration, seconds to maximum velocity
  *
  */
asynStatus devMotorAxis::move(double position, int relative, double minVelocity, double maxVelocity, double acceleration)
{
	scaleValueFromMotorRecord(&position);
	scaleValueFromMotorRecord(&maxVelocity);
    printf("Move Called to position %f\n", position);
	
	int status = putDb("AXES_1:FPOSITION", &position);
    status |= putDb("AXES_1:FVELOCITY", &maxVelocity);
    
	int exec = 1;
    status |= putDb("AXES_1:ECOMMAND", &MOVE_COMMAND);
    status |= putDb("AXES_1:BEXECUTE", &exec);

    return (asynStatus) status;
}


/** Home the motor, search the home position
  * \param[in] minimum velocity, mm/sec
  * \param[in] maximum velocity, mm/sec
  * \param[in] acceleration, seconds to maximum velocity
  * \param[in] forwards (0=backwards, otherwise forwards)
  *
  */
asynStatus devMotorAxis::home(double minVelocity, double maxVelocity, double acceleration, int forwards)
{
  asynStatus status = asynSuccess;
  printf("Home Called\n");
  return status;
}


/** jog the the motor, search the home position
  * \param[in] minimum velocity, mm/sec (not used)
  * \param[in] maximum velocity, mm/sec (positive or negative)
  * \param[in] acceleration, seconds to maximum velocity
  *
  */
asynStatus devMotorAxis::moveVelocity(double minVelocity, double maxVelocity, double acceleration)
{
  asynStatus status = asynSuccess;
  printf("Move Velo Called\n");
  return status;
}


/** Stop the axis, called by motor Record
  *
  */
asynStatus devMotorAxis::stop(double acceleration )
{
  printf("Stop Axis Called\n");
  asynStatus status = asynSuccess;
  return status;
}

double devMotorAxis::scaleValueFromMotorRecord(double* value)
{
    double mres = 0.0;
    if (pC_->getDoubleParam(axisNo_, pC_->motorResolution_, &mres) == asynSuccess && (mres != 0.0) )
    {
        *value *= mres;
    }
    return *value;
}

double devMotorAxis::scaleMotorValueToMotorRecord(double value)
{
  double mres = 0.0;
  asynStatus status = pC_->getDoubleParam(axisNo_, pC_->motorResolution_, &mres);
  if (pC_->getDoubleParam(axisNo_, pC_->motorResolution_, &mres) == asynSuccess && mres != 0.0)
  {
	  value /= mres;
  }
  return value;
}

void devMotorAxis::getDirection(int *direction) {
    int positiveDirection = 0;
    int negativeDirection = 0;
    getInteger("AXES_1:AXIS-STATUS_POSITIVEDIRECTION", &positiveDirection);
    getInteger("AXES_1:AXIS-STATUS_NEGATIVEDIRECTION", &negativeDirection);
    if (positiveDirection && negativeDirection) {
        throw std::runtime_error(std::string("Axis is running in both directions\n"));
    } else if (positiveDirection) {
        *direction = 1;
    } else if (negativeDirection) {
        *direction = 0;
    }
}

asynStatus devMotorAxis::pollAll(st_axis_status_type *pst_axis_status)
{ 
  try {
    getInteger("AXES_1:BENABLE", &pst_axis_status->bEnable);
    getInteger("AXES_1:BRESET", &pst_axis_status->bReset);
    getInteger("AXES_1:BEXECUTE", &pst_axis_status->bExecute);
    getDouble("AXES_1:FVELOCITY", &pst_axis_status->fVelocity);
    getDouble("AXES_1:FPOSITION", &pst_axis_status->fPosition);
    getDouble("AXES_1:FACCELERATION", &pst_axis_status->fAcceleration);
    getDouble("AXES_1:FDECELERATION", &pst_axis_status->fDecceleration);
    getInteger("AXES_1:BJOGFORWARD", &pst_axis_status->bJogFwd);
    getInteger("AXES_1:BJOGBACKWARD", &pst_axis_status->bJogBwd);
    getInteger("FWLIMIT_1", &pst_axis_status->bLimitFwd);
    getInteger("BWLIMIT_1", &pst_axis_status->bLimitBwd);
    getDouble("AXES_1:FOVERRIDE", &pst_axis_status->fOverride);
    getInteger("AXES_1:BERROR", &pst_axis_status->bError);
    getInteger("AXES_1:IERRORID", &pst_axis_status->nErrorId);
    getDouble("AXES_1:AXIS-NCTOPLC_ACTPOS", &pst_axis_status->fActPosition);
    getDouble("AXES_1:AXIS-NCTOPLC_ACTVELO", &pst_axis_status->fActVelocity);
    getInteger("AXES_1:AXIS-STATUS_HOMED", &pst_axis_status->bHomed);
    getDirection(&pst_axis_status->bDirection);
    //&pst_axis_status->bHomeSensor,    /* 15 */
     //&pst_axis_status->bEnabled,       /* 16 */
         //&pst_axis_status->fActDiff,       /* 21 */
    getInteger("AXES_1:BBUSY", &pst_axis_status->bBusy);
  } catch (const std::runtime_error& e) {
    printf(e.what());
    return asynError;
  }

  return asynSuccess;
}

asynStatus devMotorAxis::putDb(char *pname, const void *value) {
    DBADDR addr;
    if (dbNameToAddr(pname, &addr)) {
		printf("Couldn't find PV: %s\n", pname);
        return asynError;
    }
    long blah = dbPutField(&addr, addr.dbr_field_type, value, 1);
	printf("Error code for putting to %s: %i\n", pname, blah);
  
    return asynSuccess;
}

void devMotorAxis::getPVValue(char *pname, DBADDR* addr, long* pbuffer) {
    long options = 0;
    long no_elements;
    
    if (dbNameToAddr(pname, addr)) {
        throw std::runtime_error(std::string("PV not found: ") + pname + std::string("\n"));
    }
      
    no_elements = MIN(addr->no_elements, 100/addr->field_size);
    long status = dbGet(addr, addr->dbr_field_type, pbuffer, &options, &no_elements, NULL); 
}

void devMotorAxis::getDouble(char *pname, epicsFloat64* pvalue) {
    long buffer[100];
    long *pbuffer=&buffer[0];
    DBADDR addr;
    
    getPVValue(pname, &addr, pbuffer);
    
    if (addr.dbr_field_type == DBR_DOUBLE) {
        *pvalue = (*(epicsFloat64 *) pbuffer);
    }
}

void devMotorAxis::getInteger(char *pname, epicsInt32* pvalue) {
    long buffer[100];
    long *pbuffer=&buffer[0];
    DBADDR addr;
    
    getPVValue(pname, &addr, pbuffer);
    if (addr.dbr_field_type == DBR_LONG) {
        *pvalue = (*(epicsInt32 *) pbuffer);
    } else if (addr.dbr_field_type == DBR_ENUM) {
        *pvalue = (*(epicsEnum16 *) pbuffer);
    }
}


/** Polls the axis.
  * This function reads the motor position, the limit status, the home status, the moving status,
  * and the drive power-on status.
  * It calls setIntegerParam() and setDoubleParam() for each item that it polls,
  * and then calls callParamCallbacks() at the end.
  * \param[out] moving A flag that is set indicating that the axis is moving (true) or done (false). */
asynStatus devMotorAxis::poll(bool *moving)
{
    asynStatus comStatus;
    int nowMoving = 0;
    st_axis_status_type st_axis_status;

    memset(&st_axis_status, 0, sizeof(st_axis_status));
    
    // Go and get all the values from the device
    comStatus = pollAll(&st_axis_status);
    if (comStatus) {
        asynPrint(pC_->pasynUserController_, ASYN_TRACE_ERROR|ASYN_TRACEIO_DRIVER,
              "out=%s in=%s return=%s (%d)\n",
              pC_->outString_, pC_->inString_,
              pasynManager->strStatus(comStatus), (int)comStatus);
        return comStatus;
    }
    
    // Set the MSTA bits
    setIntegerParam(pC_->motorStatusHomed_, st_axis_status.bHomed);
    setIntegerParam(pC_->motorStatusProblem_, st_axis_status.bError);
    setIntegerParam(pC_->motorStatusLowLimit_, !st_axis_status.bLimitBwd);
    setIntegerParam(pC_->motorStatusHighLimit_, !st_axis_status.bLimitFwd);
    setIntegerParam(pC_->motorStatusPowerOn_, 1);
    setIntegerParam(pC_->motorStatusAtHome_, 0);
    setIntegerParam(pC_->motorStatusDirection_, st_axis_status.bDirection);
    
    // Get the actual position
    setDoubleParam(pC_->motorPosition_, scaleMotorValueToMotorRecord(st_axis_status.fActPosition));
    
    // Calculate if moving and set appropriate bits
    nowMoving = st_axis_status.bBusy;
    setIntegerParam(pC_->motorStatusMoving_, nowMoving);
    setIntegerParam(pC_->motorStatusDone_, !nowMoving);
    *moving = nowMoving ? true : false;
    
    callParamCallbacks();
    
    return comStatus;
}
