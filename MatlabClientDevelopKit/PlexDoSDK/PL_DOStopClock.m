function [result] = PL_DOStopClock(deviceNumber, lineNumber)
%
% [result] = PL_DOStopClock(deviceNumber, lineNumber)
%
% Stops the clock output on the specified line. Frequency and duty 
% cycle are as specified by a previous call to PL_DOSetClockParams.  
% If PL_DOSetClockParams has not been called previously, a default clock with 
% frequency = 1 kHz and duty cycle = 50% is output.  
%
% INPUT:
%   deviceNumber - NI device number returned by PL_DOGetDigitalOutputInfo
%   lineNumber - output line number (1-based)
%
% OUTPUT:
%   result - 0 if successful, -1 if NIDAQ is not installed.
%
% Copyright (c) 2005-2006, Plexon Inc
%

[result] = mexPlexDO(14, deviceNumber, lineNumber);
