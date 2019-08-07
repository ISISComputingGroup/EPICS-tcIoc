/*
FILENAME...   devMotor.h
*/

#include "asynMotorController.h"
#include "asynMotorAxis.h"
#include "dbAccess.h"

// No controller-specific parameters yet
#define NUM_VIRTUAL_MOTOR_PARAMS 0  

extern "C" {
  int devMotorCreateAxis(const char *devMotorName, int axisNo);
}

typedef struct {
  int bEnable;           /*  1 */
  int bReset;            /*  2 */
  int bExecute;          /*  3 */
  int nCommand;          /*  4 */
  int nCmdData;          /*  5 */
  double fVelocity;      /*  6 */
  double fPosition;      /*  7 */
  double fAcceleration;  /*  8 */
  double fDecceleration; /*  9 */
  int bJogFwd;           /* 10 */
  int bJogBwd;           /* 11 */
  int bLimitFwd;         /* 12 */
  int bLimitBwd;         /* 13 */
  double fOverride;      /* 14 */
  int bHomeSensor;       /* 15 */
  int bEnabled;          /* 16 */
  int bError;            /* 17 */
  int nErrorId;          /* 18 */
  double fActVelocity;   /* 19 */
  double fActPosition;   /* 20 */
  double fActDiff;       /* 21 */
  int bHomed;            /* 22 */
  int bBusy;             /* 23 */
  int bDirection;
} st_axis_status_type;

class epicsShareClass devMotorAxis : public asynMotorAxis
{
public:
	/* These are the methods we override from the base class */
	devMotorAxis(class devMotorController *pC, int axisNo);
	asynStatus move(double position, int relative, double min_velocity, double max_velocity, double acceleration);
	asynStatus moveVelocity(double min_velocity, double max_velocity, double acceleration);
	asynStatus home(double min_velocity, double max_velocity, double acceleration, int forwards);
	asynStatus stop(double acceleration);
	asynStatus pollAll(st_axis_status_type *pst_axis_status);
	asynStatus poll(bool *moving);

private:
	devMotorController *pC_;          /**< Pointer to the asynMotorController to which this axis belongs.
                                   *   Abbreviated because it is used very frequently */
	void getPVValue(char *pname, DBADDR* addr, long* pbuffer);
	void getDouble(char *pname, epicsFloat64* value);
	void getInteger(char *pname, epicsInt32* value);
	void getDirection(int *direction);
	double scaleValueFromMotorRecord(double *value);
	double scaleMotorValueToMotorRecord(double value);
	asynStatus putDb(char *pname, const void *value);
	int sendCommand(int command);
	friend class devMotorController;
	
	const epicsInt32 STOP_COMMAND = 15;
	const epicsInt32 MOVE_COMMAND = 17;
};

class epicsShareClass devMotorController : public asynMotorController {
public:
	devMotorController(const char *portName, const char *devMotorPortName, int numAxes);

	void report(FILE *fp, int level);
	asynStatus writeReadOnErrorDisconnect(void);
	devMotorAxis* getAxis(asynUser *pasynUser);
	devMotorAxis* getAxis(int axisNo);
	protected:
	void handleStatusChange(asynStatus status);
	asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

	friend class devMotorAxis;
};
