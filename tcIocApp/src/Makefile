TOP=../..

include $(TOP)/configure/CONFIG

SRC_DIRS += $(TOP)
EXPAT = $(TOP)/Expat
ADSROOT = C:/TwinCAT/AdsApi/TcAdsDll

ifneq ($(findstring windows,$(EPICS_HOST_ARCH)),)
EXPATLIB = $(EXPAT)/win64
ADSLIBROOT = $(ADSROOT)/x64
else
EXPATLIB = $(EXPAT)/win32
ADSLIBROOT = $(ADSROOT)
endif

ifneq ($(findstring debug,$(EPICS_HOST_ARCH)),)
EXPATLIB := $(EXPATLIB)/Debug
else
EXPATLIB := $(EXPATLIB)/Release
endif

ifneq ($(findstring static,$(EPICS_HOST_ARCH)),)
USR_CPPFLAGS += -DXML_STATIC
EXPATLIB := $(EXPATLIB)/libexpatMT
LIB_INSTALLS_WIN32 += $(EXPATLIB).lib
else
EXPATLIB := $(EXPATLIB)/libexpat
BIN_INSTALLS_WIN32 += $(EXPATLIB).dll
LIB_INSTALLS_WIN32 += $(EXPATLIB).lib
endif

ADSLIB = $(ADSLIBROOT)/lib/TcAdsDll
USR_DBDFLAGS += -I$(TOP)/TCatDeviceSupport
#USR_DBDFLAGS += -I$(TOP)/InfoDeviceSupport
USR_CPPFLAGS += -I$(EXPAT) -I$(ADSROOT)/Include

TYPLIBSRC = ParseTpy.cpp ParseUtil.cpp stringcase.cpp
EPICSDBLIBSRC = TpyToEpics.cpp

LIBRARY_IOC += tcIocSupport

#tcIocSupport_SRCS += devInfo.cpp
#tcIocSupport_SRCS += drvInfo.cpp
tcIocSupport_SRCS += devTc.cpp
tcIocSupport_SRCS += drvTc.cpp
tcIocSupport_SRCS += infoPlc.cpp
tcIocSupport_SRCS += plcBase.cpp
tcIocSupport_SRCS += tcComms.cpp
tcIocSupport_SRCS += $(EPICSDBLIBSRC)
tcIocSupport_SRCS += $(TYPLIBSRC)
tcIocSupport_LIBS += $(EPICS_BASE_IOC_LIBS)
tcIocSupport_SYS_LIBS_WIN32 += $(EXPATLIB)
tcIocSupport_SYS_LIBS_WIN32 += $(ADSLIB)

PROD_IOC = tpyinfo epicsdbgen tcIoc

DBD += tcIocSupport.dbd tcIoc.dbd
tcIoc_DBD += base.dbd
tcIoc_DBD += tcIocSupport.dbd

tpyinfo_SRCS += ParseTpyInfo.cpp
tpyinfo_SRCS += $(TYPLIBSRC)
tpyinfo_SYS_LIBS_WIN32 += $(EXPATLIB)

epicsdbgen_SRCS += EpicsDbGen.cpp
epicsdbgen_SRCS += $(EPICSDBLIBSRC)
epicsdbgen_SRCS += $(TYPLIBSRC)
epicsdbgen_SYS_LIBS_WIN32 += $(EXPATLIB)

# tcIoc_registerRecordDeviceDriver.cpp derives from tcIoc.dbd
tcIoc_SRCS += iocMain.cpp
tcIoc_SRCS += tcIoc_registerRecordDeviceDriver.cpp

# Finally link to the EPICS Base libraries
tcIoc_LIBS += tcIocSupport
tcIoc_LIBS += $(EPICS_BASE_IOC_LIBS)
tcIoc_SYS_LIBS_WIN32 += $(EXPATLIB)
tcIoc_SYS_LIBS_WIN32 += $(ADSLIB)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

