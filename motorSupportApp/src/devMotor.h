#include "asynMotorController.h"
#include "asynMotorAxis.h"
#include "dbAccess.h"

// No controller-specific parameters yet
#define NUM_VIRTUAL_MOTOR_PARAMS 0  

#define PV_BUFFER_LEN 100
#define MOTOR_SCALING_FACTOR 0.0001

extern "C" {
	int devMotorCreateAxis(const char *devMotorName, int axisNo, int versionNumber);
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

protected:
	void getInteger(std::string pvSuffix, epicsInt32* pvalue, const std::string* prefix = 0);
	asynStatus sendCommand(const int command);    
	asynStatus putDb(std::string pvSuffix, const void *value);
	
    int axisNo;
	devMotorController *pC_;
	std::string pvPrefix;
	
	friend class devMotorController;
	
private:
	void getPVValue(std::string& pvSuffix, DBADDR* addr, long* pbuffer, const std::string* prefix = 0);
	void getDouble(std::string pvSuffix, epicsFloat64* pvalue);
	void getDirection(int* direction);
	void scaleValueFromMotorRecord(double* value);
	void scaleValueToMotorRecord(double* value);
	
    virtual void populateLimitStatus(st_axis_status_type *axis_status) = 0;
	virtual asynStatus sendStop() = 0;
		
	std::string previousError = "";
	bool errorToggle = false;

    virtual std::string ENABLE_STATUS() = 0;
	virtual std::string EXECUTE() = 0;
	virtual std::string VELOCITY_SP() = 0;
	virtual std::string POSITION_SP() = 0;
	virtual std::string DISTANCE_SP() = 0;
	virtual std::string ERROR_STATUS() = 0;
	virtual std::string POSITION_RBV() = 0;
	virtual std::string VELOCITY_RBV() = 0;
	virtual std::string HOMED() = 0;
	virtual std::string MOVING() = 0;
	virtual std::string COMMAND() = 0;
	virtual std::string POSITIVE_DIR() = 0;
	virtual std::string NEGATIVE_DIR() = 0;

	
	virtual epicsInt32 HOME_COMMAND() = 0;
	virtual epicsInt32 MOVE_ABS_COMMAND() = 0;
	virtual epicsInt32 MOVE_RELATIVE_COMMAND() = 0;
	virtual epicsInt32 MOVE_VELO_COMMAND() = 0;
};

class epicsShareClass twincatMotorAxis : public devMotorAxis
{
public: 
    twincatMotorAxis(class devMotorController *pC, int axisNo): devMotorAxis(pC, axisNo) {};

private:
	std::string ENABLE_STATUS() { return "STSTATUS-BENABLED"; };
	std::string EXECUTE() { return "STCONTROL-BEXECUTE"; };
	std::string STOP() { return "STCONTROL-BSTOP"; };
	std::string VELOCITY_SP() { return "STCONTROL-FVELOCITY"; };
	std::string POSITION_SP() { return "STCONTROL-FPOSITION"; };
	std::string DISTANCE_SP() { return "STCONTROL-FPOSITION"; };
	std::string ERROR_STATUS() { return "STSTATUS-BERROR"; };
	std::string POSITION_RBV() { return "STSTATUS-FACTPOSITION"; };
	std::string VELOCITY_RBV() { return "STSTATUS-FACTVELOCITY"; };
	std::string HOMED() { return "STSTATUS-BHOMED"; };
	std::string MOVING() { return "STSTATUS-BMOVING"; };
	std::string COMMAND() { return "STCONTROL-ECOMMAND"; };
	std::string POSITIVE_DIR() { return "STSTATUS-BMOVINGFORWARD"; };
	std::string NEGATIVE_DIR() { return "STSTATUS-BMOVINGBACKWARD"; };
	
    epicsInt32 HOME_COMMAND() { return 10; };
	epicsInt32 STOP_COMMAND() { return 15; };
	epicsInt32 MOVE_ABS_COMMAND() { return 0; };
	epicsInt32 MOVE_RELATIVE_COMMAND() { return 1; };
	epicsInt32 MOVE_VELO_COMMAND() { return 3; };

    void populateLimitStatus(st_axis_status_type *axis_status);
	asynStatus sendStop();
};

class epicsShareClass ISISMotorAxis : public devMotorAxis
{
public: 
    ISISMotorAxis(class devMotorController *pC, int axisNo): devMotorAxis(pC, axisNo) {};

private:
    std::string ENABLE_STATUS() { return "BENABLE"; };
	std::string EXECUTE() { return "BEXECUTE"; };
	std::string VELOCITY_SP() { return "FVELOCITY"; };
	std::string POSITION_SP() { return "FPOSITION"; };
	std::string DISTANCE_SP() { return "FDISTANCE"; };
	std::string ERROR_STATUS() { return "BERROR"; };
	std::string POSITION_RBV() { return "FACTPOSITION"; };
	std::string VELOCITY_RBV() { return "FACTVELOCITY"; };
	std::string HOMED() { return "BCALIBRATED"; };
	std::string POSITIVE_DIR() { return "BPOSITIVEDIRECTION"; };
	std::string NEGATIVE_DIR() { return "BNEGATIVEDIRECTION"; };
	std::string MOVING() { return "BMOVING"; };
	std::string COMMAND() { return "ECOMMAND"; };
	
	epicsInt32 HOME_COMMAND() { return 13; };
	epicsInt32 STOP_COMMAND() { return 15; };
	epicsInt32 MOVE_ABS_COMMAND() { return 17; };
	epicsInt32 MOVE_RELATIVE_COMMAND() { return 18; };
	epicsInt32 MOVE_VELO_COMMAND() { return 21; };

    void populateLimitStatus(st_axis_status_type *axis_status);
	asynStatus sendStop();
};

class epicsShareClass devMotorController : public asynMotorController {
public:
	devMotorController(const char *portName, const char *devMotorPortName, int numAxes, const char *pvPrefix);

	void report(FILE *fp, int level);

	friend class devMotorAxis;
	std::string pvPrefix;
};
