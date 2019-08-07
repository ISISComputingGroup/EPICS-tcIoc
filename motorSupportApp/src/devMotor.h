#include "asynMotorController.h"
#include "asynMotorAxis.h"
#include "dbAccess.h"

// No controller-specific parameters yet
#define NUM_VIRTUAL_MOTOR_PARAMS 0  

extern "C" {
	int devMotorCreateAxis(const char *devMotorName, int axisNo);
}

typedef struct {
    int bEnable;                      
    int bExecute;                   
    double fVelocity;      
    double fPosition;         
    int bLimitFwd;         
    int bLimitBwd;                           
    int bError;                     
    double fActVelocity;   
    double fActPosition;   
    int bHomed;            
    int bMoving;           
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
	double getMotorResolution();
	void scaleValueFromMotorRecord(double* value);
	void scaleValueToMotorRecord(double* value);
	asynStatus putDb(char *pname, const void *value);
	int sendCommand(int command);
	friend class devMotorController;
	
	const epicsInt32 STOP_COMMAND = 15;
	const epicsInt32 MOVE_ABS_COMMAND = 17;
	const epicsInt32 MOVE_RELATIVE_COMMAND = 18;
	const epicsInt32 MOVE_VELO_COMMAND = 21;
};

class epicsShareClass devMotorController : public asynMotorController {
public:
	devMotorController(const char *portName, const char *devMotorPortName, int numAxes);

	void report(FILE *fp, int level);
	devMotorAxis* getAxis(asynUser *pasynUser);
	devMotorAxis* getAxis(int axisNo);

protected:
	friend class devMotorAxis;
};
