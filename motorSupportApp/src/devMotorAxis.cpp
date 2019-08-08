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
#define MIN(x,y)  (((x) < (y)) ? (x) : (y))
#endif

/** Creates a new devMotorAxis object.
  * \param[in] pC Pointer to the EssMCAGmotorController to which this axis belongs.
  * \param[in] axisNo Index number of this axis, range 1 to pC->numAxes_. (0 is not used)
  *
  *
  * Initializes register numbers, etc.
  */
devMotorAxis::devMotorAxis(devMotorController *pC, int axisNo) 
    : asynMotorAxis(pC, axisNo), pC_(pC), axisNo(axisNo)
{
    printf("Axis created\n");
	pvPrefix = "AXES_" + std::to_string(axisNo + 1) + ":";
    pC_->wakeupPoller();
}


extern "C" int devMotorCreateAxis(const char *devMotorName, int axisNo) {
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

int devMotorAxis::sendCommand(int command) {
    int exec = 1;
    int status = putDb("ECOMMAND", &command);
    status |= putDb("BEXECUTE", &exec);
    return status;
}

/** Move the axis to a position, either absolute or relative
  * \param[in] position in mm
  * \param[in] relative (0=absolute, otherwise relative)
  * \param[in] minimum velocity, mm/sec
  * \param[in] maximum velocity, mm/sec
  * \param[in] acceleration, seconds to maximum velocity
  *
  */
asynStatus devMotorAxis::move(double position, int relative, double minVelocity, double maxVelocity, double acceleration) {
    scaleValueFromMotorRecord(&position);
    scaleValueFromMotorRecord(&maxVelocity);
    printf("Move Called to position %f\n", position);
    
    int status = putDb("FPOSITION", &position);
    status |= putDb("FVELOCITY", &maxVelocity);
    
    status |= sendCommand(MOVE_ABS_COMMAND);

    return (asynStatus) status;
}


/** Home the motor, search the home position
  * \param[in] minimum velocity, mm/sec
  * \param[in] maximum velocity, mm/sec
  * \param[in] acceleration, seconds to maximum velocity
  * \param[in] forwards (0=backwards, otherwise forwards)
  *
  */
asynStatus devMotorAxis::home(double minVelocity, double maxVelocity, double acceleration, int forwards) {
    printf("Home Called\n");
    return asynSuccess;
}


/** jog the the motor
  * \param[in] minimum velocity, mm/sec (not used)
  * \param[in] maximum velocity, mm/sec (positive or negative)
  * \param[in] acceleration, seconds to maximum velocity
  *
  */
asynStatus devMotorAxis::moveVelocity(double minVelocity, double maxVelocity, double acceleration) {
    try {
		scaleValueFromMotorRecord(&maxVelocity);
		
		int status = putDb("FVELOCITY", &maxVelocity);
		status |= sendCommand(MOVE_VELO_COMMAND);
		return (asynStatus)status;
    }  catch (const std::runtime_error& e) {
		asynPrint(pC_->pasynUserController_, ASYN_TRACE_ERROR|ASYN_TRACEIO_DRIVER,
					"Failed to move velocity axis %i: %s\n", axisNo, e.what());
		return asynError;
	}
}

/** 
  * Stops the axis, called by motor record.
  * 
  * @param acceleration The acceleration to stop the axis with (currently unused).
  */
asynStatus devMotorAxis::stop(double acceleration) {
    try {
		return (asynStatus)sendCommand(STOP_COMMAND);
	}  catch (const std::runtime_error& e) {
		asynPrint(pC_->pasynUserController_, ASYN_TRACE_ERROR|ASYN_TRACEIO_DRIVER,
					"Failed to stop axis %i: %s\n", axisNo, e.what());
		return asynError;
	}
}

/**
  * Gets the motor resolution inside the motor record.
  * 
  * @return The motor resolution or 1.0 if no resolution can be found.
  */
double devMotorAxis::getMotorResolution() {
    double mres = 1.0;
    if (pC_->getDoubleParam(axisNo_, pC_->motorResolution_, &mres) == asynSuccess) {
        return mres;
    } else {
        return 1.0;
    }
}

/**
  * Scales the value coming from the motor record towards the PLC.
  *
  * The PLC works in real world units the scaling will need to be but
  * the motor record works in steps, the resolution is the scaling.
  *
  * @param value A pointer to the value that is being scaled.
  */
void devMotorAxis::scaleValueFromMotorRecord(double* value) {
    *value *= getMotorResolution();
}

/**
  * Scales the value coming from the PLC towards the motor record.
  *
  * The PLC works in real world units the scaling will need to be but
  * the motor record works in steps, the resolution is the scaling.
  *
  * @param value A pointer to the value that is being scaled.
  */
void devMotorAxis::scaleValueToMotorRecord(double* value) {
    *value /= getMotorResolution();
}

void devMotorAxis::getDirection(int *direction) {
    int positiveDirection = 0;
    int negativeDirection = 0;
    getInteger("AXIS-STATUS_POSITIVEDIRECTION", &positiveDirection);
    getInteger("AXIS-STATUS_NEGATIVEDIRECTION", &negativeDirection);
    if (positiveDirection && negativeDirection) {
        throw std::runtime_error(std::string("Axis is running in both directions\n"));
    } else if (positiveDirection) {
        *direction = 1;
    } else if (negativeDirection) {
        *direction = 0;
    }
}

asynStatus devMotorAxis::pollAll(st_axis_status_type *pst_axis_status) { 
    try {
        getInteger("BENABLE", &pst_axis_status->bEnable);
        getInteger("BEXECUTE", &pst_axis_status->bExecute);
        getDouble("FVELOCITY", &pst_axis_status->fVelocity);
        getDouble("FPOSITION", &pst_axis_status->fPosition);
        getInteger("FWLIMIT_" + std::to_string(axisNo + 1), &pst_axis_status->bLimitFwd, &std::string("")); 
        getInteger("BWLIMIT_" + std::to_string(axisNo + 1), &pst_axis_status->bLimitBwd, &std::string(""));
        getInteger("BERROR", &pst_axis_status->bError);
        getDouble("AXIS-NCTOPLC_ACTPOS", &pst_axis_status->fActPosition);
        getDouble("AXIS-NCTOPLC_ACTVELO", &pst_axis_status->fActVelocity);
        getInteger("AXIS-STATUS_HOMED", &pst_axis_status->bHomed);
        getInteger("BMOVING", &pst_axis_status->bMoving);
        getDirection(&pst_axis_status->bDirection);
    } catch (const std::runtime_error& e) {
		asynPrint(pC_->pasynUserController_, ASYN_TRACE_ERROR|ASYN_TRACEIO_DRIVER,
					"Failed to poll controller for axis %i: %s\n", axisNo, e.what());
        return asynError;
    }
    return asynSuccess;
}

asynStatus devMotorAxis::putDb(std::string pvSuffix, const void *value) {
    DBADDR addr;
	std::string fullPV = pvPrefix + pvSuffix;
    if (dbNameToAddr(fullPV.c_str(), &addr)) {
		throw std::runtime_error("PV not found: " + fullPV + "\n");
        return asynError;
    }

    return (asynStatus) dbPutField(&addr, addr.dbr_field_type, value, 1);
}

void devMotorAxis::getPVValue(std::string& pvSuffix, DBADDR* addr, long* pbuffer, const std::string* prefix) {
    long options = 0;
    long no_elements;
    
	if (!prefix) {
		prefix = &pvPrefix;
	}
	
	std::string fullPV = *prefix + pvSuffix;
    if (dbNameToAddr(fullPV.c_str(), addr)) {
        throw std::runtime_error("PV not found: " + fullPV + "\n");
    }
      
    no_elements = MIN(addr->no_elements, 100/addr->field_size);
    if (dbGet(addr, addr->dbr_field_type, pbuffer, &options, &no_elements, NULL) != 0) {
        throw std::runtime_error("Could not get value from PV: " + fullPV + "\n");
    }
}

void devMotorAxis::getDouble(std::string pvSuffix, epicsFloat64* pvalue) {
    long buffer[100];
    long *pbuffer=&buffer[0];
    DBADDR addr;

    getPVValue(pvSuffix, &addr, pbuffer);
    
    if (addr.dbr_field_type == DBR_DOUBLE) {
        *pvalue = (*(epicsFloat64 *) pbuffer);
    }
}

void devMotorAxis::getInteger(std::string pvSuffix, epicsInt32* pvalue, const std::string* prefix) {
    long buffer[100];
    long *pbuffer=&buffer[0];
    DBADDR addr;
    
    getPVValue(pvSuffix, &addr, pbuffer, prefix);
    
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
asynStatus devMotorAxis::poll(bool *moving) {
    asynStatus comStatus;
    int nowMoving = 0;
    st_axis_status_type st_axis_status;

    memset(&st_axis_status, 0, sizeof(st_axis_status));
    
    // Go and get all the values from the device
    comStatus = pollAll(&st_axis_status);
    if (comStatus) {
        return comStatus;
    }
    
    // Set the MSTA bits
    setIntegerParam(pC_->motorStatusHomed_, st_axis_status.bHomed);
    setIntegerParam(pC_->motorStatusProblem_, st_axis_status.bError);
    setIntegerParam(pC_->motorStatusLowLimit_, !st_axis_status.bLimitBwd);
    setIntegerParam(pC_->motorStatusHighLimit_, !st_axis_status.bLimitFwd);
    setIntegerParam(pC_->motorStatusPowerOn_, st_axis_status.bEnable);
    setIntegerParam(pC_->motorStatusAtHome_, 0);
    setIntegerParam(pC_->motorStatusDirection_, st_axis_status.bDirection);
    
    // Get the actual position
    scaleValueToMotorRecord(&st_axis_status.fActPosition);
    setDoubleParam(pC_->motorPosition_, st_axis_status.fActPosition);
    
    // Calculate if moving and set appropriate bits
    nowMoving = st_axis_status.bMoving;
    setIntegerParam(pC_->motorStatusMoving_, nowMoving);
    setIntegerParam(pC_->motorStatusDone_, !nowMoving);
    *moving = nowMoving ? true : false;
    
    callParamCallbacks();
    
    return comStatus;
}
