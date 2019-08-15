#include "asynMotorController.h"
#include "asynMotorAxis.h"
#include "dbAccess.h"

// No controller-specific parameters yet
#define NUM_VIRTUAL_MOTOR_PARAMS 0  

#define PV_BUFFER_LEN 100

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
	void getPVValue(std::string& pvSuffix, DBADDR* addr, long* pbuffer, const std::string* prefix = 0);
	void getDouble(std::string pvSuffix, epicsFloat64* pvalue);
	void getInteger(std::string pvSuffix, epicsInt32* pvalue, const std::string* prefix = 0);
	void getDirection(int *direction);
	double getMotorResolution();
	void scaleValueFromMotorRecord(double* value);
	void scaleValueToMotorRecord(double* value);
	asynStatus putDb(std::string pvSuffix, const void *value);
	asynStatus sendCommand(const int command);
	
	friend class devMotorController;
	
	int axisNo;
	asynStatus previousComStatus = asynSuccess;
	bool errorToggle = false;

	const epicsInt32 HOME_COMMAND = 13;
	const epicsInt32 STOP_COMMAND = 15;
	const epicsInt32 MOVE_ABS_COMMAND = 17;
	const epicsInt32 MOVE_RELATIVE_COMMAND = 18;
	const epicsInt32 MOVE_VELO_COMMAND = 21;
	
	std::string pvPrefix;
};

class epicsShareClass devMotorController : public asynMotorController {
public:
	devMotorController(const char *portName, const char *devMotorPortName, int numAxes, const char *pvPrefix);

	void report(FILE *fp, int level);

protected:
	friend class devMotorAxis;
	std::string pvPrefix;
};
