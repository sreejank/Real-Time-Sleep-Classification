function [result] = PL_DOStartClock(deviceNumber, lineNumber)
%
% [result] = PL_DOStartClock(deviceNumber, lineNumber)
%
% Starts the clock output on the specified line. Frequency and duty 
% cycle are as specified by a previous call to PL_DOSetClockParams.  
% If PL_DOSetClockParams has not been called previously, a default clock with 
% frequency = 1 kHz and duty cycle = 50% is output.  
%
% INPUT:
%   deviceNumber - NI device number returned by PL_DOGetDigitalOutputInfo.m
%   lineNumber - output line number (1-based)
%
% OUTPUT:
%   result - 0 if successful, -1 if NIDAQ is not installed.
%
% Copyright (c) 2005, Plexon Inc
%

[result] = mexPlexDO(13, deviceNumber, lineNumber);
