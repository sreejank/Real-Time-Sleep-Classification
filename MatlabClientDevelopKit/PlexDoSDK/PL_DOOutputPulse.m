function [result] = PL_DOOutputPulse(deviceNumber, lineNumber)
%
% [result] = PL_DOOutputPulse(deviceNumber, lineNumber)
%
% Outputs a single pulse on the specified line.  Duration is as specified 
% by a previous call to PL_DOSetPulseDuration. If PL_DOSetPulseDuration has 
% not been called previously, a default pulse width of 1 msec is used.
%
% INPUT:
%   deviceNumber - NI device number returned by PL_DOGetDigitalOutputInfo.m
%   lineNumber - output line number (1-based)
%
% OUTPUT:
%   result - 0 if successful, -1 if NIDAQ is not installed.
%
% Copyright (c) 2005-2006, Plexon Inc
%

[result] = mexPlexDO(11, deviceNumber, lineNumber);
