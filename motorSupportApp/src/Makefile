TOP=../..

include $(TOP)/configure/CONFIG

LIBRARY_IOC += tcIocMotorSupport

tcIocMotorSupport_SRCS += devMotorController.cpp
tcIocMotorSupport_SRCS += devMotorAxis.cpp

DBD += tcIocMotorSupport.dbd
tcIocMotorSupport_DBD += devMotorController.dbd

# Finally link to the EPICS Base libraries
tcIocMotorSupport_LIBS += asyn
tcIocMotorSupport_LIBS += motor
tcIocMotorSupport_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

