function [result] = PL_DOSleep(millisecs)
%
% [result] = PL_DOSleep(millisecs)
%
% Sleeps for the specified time (in milliseconds) before returning.  
% Better timing accuracy than the Win32 Sleep() function, but note that 
% accuracy can vary, depending on system activity.  For precision pulse 
% output, a digital output line and the functions PL_DOSetPulseDuration and 
% PL_DOOutputPulse should be used, rather than timing individual pulses 
% using PL_DOSleep.
%
% INPUT:
%   millisecs - time in milliseconds
%
% OUTPUT:
%   result - 0 if successful, -1 if NIDAQ is not installed.
%
% Copyright (c) 2005, Plexon Inc
%

[result] = mexPlexDO(15, millisecs);
