#!../../bin/win32-x86/tcIoc

## You may have to change tcIoc to something else
## everywhere it appears in this file

# Increase this if you get <<TRUNCATED>> or discarded messages warnings in your errlog output
errlogInit2(65536, 256)

< envPaths

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/tcIoc.dbd"
tcIoc_registerRecordDeviceDriver pdbbase

## calling common command file in ioc 01 boot dir
cd ${TOP}/iocBoot/ioctcIoc
< st-common.cmd
