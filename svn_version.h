#pragma once
 
/// True, if there are modifications to the local working copy, false otherwise
const bool 			svn_local_modifications = false;
/// Highest committed revision number in the working copy
const int 			svn_revision = 1;
/// Current system date &amp; time     
const char* const 	svn_time_now = "$WCNOW$";
/// Revision number if fully committed, 0 otherwise
const int			svn_revision_committed = 1;
