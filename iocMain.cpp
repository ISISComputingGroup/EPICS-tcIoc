#include "epicsExit.h"
#include "epicsThread.h"
#include "iocsh.h"
#include "svn_version.h"
#include <ctime>

extern "C" {
__declspec(dllimport) void stopTc(void); ///< DLL import for stopTc
}



// Time the initialization process
static clock_t begin;
static clock_t end;

/** @file iocMain.cpp
	The main program for the TwinCAT IOC.
 ************************************************************************/


static bool debug;

/** Main program for tcIoc
	@brief tcIoc
 */
int main(int argc,char *argv[])
{
	// Start timer
	begin = clock();
	printf("Subversion: Committed %s   Revision %i   Date %s.\n",
		   svn_local_modifications ? "no" : "yes", svn_revision_committed, svn_time_now);

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

