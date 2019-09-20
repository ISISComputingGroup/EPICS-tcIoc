cd %~dp0
SubWCRev.exe . .\svn_version.h.tmpl .\svn_version.h
IF ERRORLEVEL 1 copy .\svn_version.h.default .\svn_version.h
