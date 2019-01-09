#include "epicsExit.h"
#include "epicsThread.h"
#include "iocsh.h"
#include <ctime>

extern "C" {
__declspec(dllimport) void stopTc(void);
}



// Time the initialization process
clock_t begin;
clock_t end;

/** @file iocMain.cpp
	The main program for the TwinCAT IOC.
 ************************************************************************/


bool debug;

int main(int argc,char *argv[])
{
	// Start timer
	begin = clock();

	// Run initialization
	if(argc>=2) {    
        iocsh(argv[1]);
        epicsThreadSleep(.2);
    }

	// Stop timer
	end = clock();
	printf("Initialization completed in %f seconds.\n",((float)(end - begin)/CLOCKS_PER_SEC));

	// EPICS command line
	iocsh(NULL);

	// Exit
	stopTc();
    epicsExit(0);

	return(0);
}

